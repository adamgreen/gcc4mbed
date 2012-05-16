/* Copyright 2011 Adam Green (http://mbed.org/users/AdamGreen/)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
/* Implementation of HTTP Server functionality. */
#include "HTTPServer.h"
#include "network.h"
#include "debug.h"


// HTTP_TRACE to 1/0 to enable/disable logging for the HTTP server.
#define HTTP_TRACE      0
#if HTTP_TRACE
    #define TRACE printf
#else
    static void __trace(...)
    {
        return;
    }
    #define TRACE __trace
#endif // HTTP_TRACE



// Maximum size of buffer to be allocated on the stack for building filename
// from URI that can be used in mbed fopen() call.
#define HTTP_MAX_FILENAME       64

// Maximum size of a request line to be buffered.
#define HTTP_MAX_REQUEST_LINE   128


// Utility macro to determine the element count of an array.
#define ARRAYSIZE(X) (sizeof(X)/sizeof(X[0]))



// Default filenames to be used when the requested URI is "/"
static const char* g_DefaultFilenames[] =
{
    "/index.html",
    "/index.htm"
};



// Static-Scope Function Prototypes.
static void        NetworkPrintRemoteAddress(tcp_pcb* pPCB);



// Structure used to represent each header to be sent.  It includes both a
// pointer to the constant string and the length of the constant string so
// that a memcpy() can be used for moving into buffer instead of strcpy().
struct SHeader
{
    // Header string.
    const char*     pHeaderString;
    // Length of header string, not including the NULL terminator.
    unsigned int    HeaderLength;
};


// Structure which stores information about each HTTP client connection.
class CHTTPContext : protected IHTTPRequestHandlerContext
{
public:
    // Constructors/Destructors
    CHTTPContext(CHTTPServer* pHTTPServer, tcp_pcb* pPCB);
    ~CHTTPContext();
    
    // IHTTPRequestHandlerContext methods.
    virtual int  BeginRequestHeaders();
    virtual void WriteRequestHeader(const char* pHeaderName, 
                                    const char* pHeaderValue);
    virtual void EndRequestHeaders();
    virtual int BeginRequestContent(size_t ContentSize);
    virtual void WriteRequestContentBlock(const void* pBlockBuffer, 
                                          size_t BlockBufferSize);
    virtual void EndRequestContent();
    virtual const char* GetStatusLine(size_t* pStatusLineLength);
    virtual const char* GetResponseHeaders(size_t* pResponseHeaderLength);
    virtual int BeginResponseContent();
    virtual size_t ReadResponseContentBlock(char*  pBuffer,
                                            size_t BytesToRead);
    virtual void EndResponseContent();
    virtual void Release();
    
    CHTTPServer*    Server() { return m_pHTTPServer; }
    CHTTPContext*   GetNext() { return m_pNext; }
    void            RemovedFromWaitingList(CHTTPContext* pPrev) 
                    { 
                        if (pPrev)
                        {
                            pPrev->m_pNext = this->m_pNext;
                        }
                        m_pNext = NULL; 
                        m_WaitingForBuffer = 0; 
                    }
    void            AddToWaitingList(CHTTPContext* pPrev)
                    {
                        if (pPrev)
                        {
                            assert ( !pPrev->m_pNext );
                            pPrev->m_pNext = this;
                        }
                        m_WaitingForBuffer = 1;
                    }
    int             IsWaitingForBuffer() { return m_WaitingForBuffer; }

    void            CloseConnection();
    void            FreeContext(int ForcedFree);
    void            BufferAvailable();

protected:
    // Static methods for lwIP callbacks.
    static err_t        HTTPRecv(void* pvArg, tcp_pcb* pPCB, pbuf* pBuf, err_t err);
    static err_t        HTTPSent(void* pvArg, tcp_pcb* pPCB, u16_t SendCount);
    static void         HTTPError(void* pvArg, err_t Error);
    static err_t        HTTPPoll(void* pvArg, tcp_pcb* pPCB);
    
    // Protected helper methods.
    void                ProcessRequestData(pbuf* pBuf);
    void                BufferRequestLines(pbuf* pBuf);
    void                ProcessRequestContent(pbuf* pBuf);
    void                ParseRequestLine();
    void                ParseFirstRequestLine();
    void                ParseHeaderRequestLine();
    void                StartRequestHandler(const char* pURI);
    void                StartGetRequest(const char* pURI);
    void                StartHeadRequest(const char* pURI);
    void                StartPostRequest(const char* pURI);
    void                StartBadRequest(const char* pRequest);
    void                OpenFile(const char* pURI);
    void                PrepareResponse();
    void                PrepareResponseHeaders();
    void                ReadContent();
    void                SendResponse();
    int                 WriteData();

    // States used to track the HTTP request parsing progress.
    enum EParseState {
        STATE_FIRST_LINE,
        STATE_HEADERS,
        STATE_CONTENT,
        STATE_DONE
    };
    
    // Type of HTTP request that has been made by the client.
    enum ERequestType
    {
        TYPE_GET_0_9,
        TYPE_GET_1_0,
        TYPE_HEAD,
        TYPE_POST,
        TYPE_BAD_REQUEST
    };

    // Pointer to parent HTTP Server object.
    CHTTPServer*    m_pHTTPServer;
    // Pointer to the tcp_pcb 'socket' for this context.
    tcp_pcb*        m_pPCB;
    // The next context in the buffer waiting queue.
    CHTTPContext*   m_pNext;
    // Handle of file to be sent to the client.
    FILE*           m_pFile;
    // Pointer to the request handler context.  May point to self for
    // default GET/HEAD request handling.
    IHTTPRequestHandlerContext* m_pRequestHandlerContext;
    // Content-Type description header field.
    const char*     m_pContentType;
    // HTML text to be sent back in response to error.
    const char*     m_pErrorHTML;
    // Queue of buffers used by this context.  BufferQueueHead and 
    // BufferQueueTail track the current buffers being written and/or
    // waiting for acknowledgment from remote machine.
    SBuffer*        m_BufferQueue[2];
    // Array of pointers to constant header strings to be sent before response
    // content.  Status Line, Server:, Other Headers, /r/n
    SHeader         m_HeaderArray[4];
    // The length of the m_pContentType string.
    size_t          m_ContentTypeLength;
    // The length of the m_pErrorHTML string.
    size_t          m_ErrorHTMLLength;
    // The current read offset into the m_pErrorHTML string.
    size_t          m_ErrorHTMLOffset;
    // UNDONE: Many of these fields could be packed into a single field to save memory.
    // Are we waiting for a buffer already?
    int             m_WaitingForBuffer;
    // Are we in the process of closing down the connection?
    int             m_ClosingConnection;
    // Should request headers be sent to handler?
    int             m_SendRequestHeadersToHandler;
    // Should request content be sent to handler?
    int             m_SendRequestContentToHandler;
    // Content-Length of request content body.
    unsigned int    m_RequestContentLength;
    // The current state of the request parsing code.
    EParseState     m_ParseState;
    // The type of request being made by the client.
    ERequestType    m_RequestType;
    // UNDONE: Check for memory savings across these structures.
    // Offset into m_LineBuffer where next character should be placed.
    unsigned short  m_LineBufferOffset;
    // Offset into current header being sent to client.
    unsigned short  m_HeaderOffset;
    // Head and tail indices for SentBufferQueue.
    // Head is the buffer currently being written.
    unsigned char   m_BufferQueueHead;
    // Tail is the buffer currently in the process of being acknowledged.
    unsigned char   m_BufferQueueTail;
    // Number of tcp_write() retries attempted so far.
    unsigned char   m_RetryCount;
    // Current index into HeaderArray[] of next header to be sent to client.
    unsigned char   m_HeaderIndex;
    // Each line of the request is buffered here.
    char            m_LineBuffer[HTTP_MAX_REQUEST_LINE+1];
};


CHTTPContext::CHTTPContext(CHTTPServer* pHTTPServer, tcp_pcb* pPCB)
{
    m_pHTTPServer = pHTTPServer;
    m_pPCB = pPCB;
    m_pNext = NULL;
    m_pFile = NULL;
    m_pRequestHandlerContext = NULL;
    m_pContentType = NULL;
    m_pErrorHTML = NULL;
    memset(m_BufferQueue, 0, sizeof(m_BufferQueue));
    memset(m_HeaderArray, 0, sizeof(m_HeaderArray));
    m_ContentTypeLength = 0;
    m_ErrorHTMLLength = 0;
    m_ErrorHTMLOffset = 0;
    m_WaitingForBuffer = 0;
    m_ClosingConnection = 0;
    m_SendRequestHeadersToHandler = 0;
    m_SendRequestContentToHandler = 0;
    m_RequestContentLength = 0;
    m_ParseState = STATE_FIRST_LINE;
    m_RequestType = TYPE_BAD_REQUEST;
    m_LineBufferOffset = 0;
    m_HeaderOffset = 0;
    m_BufferQueueHead = 0;
    m_BufferQueueTail = 0;
    m_RetryCount = 0;
    m_HeaderIndex = 0;
    m_LineBuffer[0] = '\0';

    // Associate this context with the lwIP PCB object.
    tcp_arg(pPCB, this);
    
    // Setup the callbacks to be called whenever any activity occurs on this new
    // client PCB.
    tcp_recv(pPCB, HTTPRecv);
    tcp_sent(pPCB, HTTPSent);
    tcp_err(pPCB, HTTPError);
    
    // There might be scenarios where the application can't queue up more work
    // for this client PCB (due to things like out of memory) so that activity
    // could cease on this PCB when there were no more calls to functions like
    // HTTPSent().  To make sure this doesn't happen, have lwIP callback into 
    // the HTTPPoll() function every 4 iterations of the coarse TCP timer (2 
    // seconds).  HTTPPoll() can take care of retrying any previously failed 
    // operations.
    tcp_poll(pPCB, HTTPPoll, 4);
}


// Destructor
CHTTPContext::~CHTTPContext()
{
    // Make sure that everything was cleaned up before destructor was called.
    assert ( NULL == m_pFile );
    assert ( NULL == m_pPCB );
    assert ( NULL == m_pRequestHandlerContext );
}


int  CHTTPContext::BeginRequestHeaders()
{
    // This context already parses out the Content-Length header that it needs.
    return 0;
}

void CHTTPContext::WriteRequestHeader(const char* pHeaderName, 
                                      const char* pHeaderValue)
{
    static const int ShouldNotBeCalled = 0;
    
    (void)pHeaderName;
    (void)pHeaderValue;
    
    assert ( ShouldNotBeCalled );
}

void CHTTPContext::EndRequestHeaders()
{
    static const int ShouldNotBeCalled = 0;
    
    assert ( ShouldNotBeCalled );
}

int CHTTPContext::BeginRequestContent(size_t ContentSize)
{
    (void)ContentSize;
    
    // Only processing GET and HEAD requests so don't care about content sent
    // in request.
    return 0;
}

void CHTTPContext::WriteRequestContentBlock(const void* pBlockBuffer, 
                                            size_t BlockBufferSize)
{
    static const int ShouldNotBeCalled = 0;
    
    (void)pBlockBuffer;
    (void)BlockBufferSize;
    
    assert ( ShouldNotBeCalled );
}

void CHTTPContext::EndRequestContent()
{
    static const int ShouldNotBeCalled = 0;
    
    assert ( ShouldNotBeCalled );
}

const char* CHTTPContext::GetStatusLine(size_t* pStatusLineLength)
{
    static const char Ok[] = "HTTP/1.0 200 OK\r\n";
    static const char NotFound[] = "HTTP/1.0 404 Not Found\r\n";
    static const char BadRequest[] = "HTTP/1.0 400 Bad Request\r\n";
    static const char NotImplemented[] = "HTTP/1.0 501 Not Implemented\r\n";
    
    switch (m_RequestType)
    {
    case TYPE_HEAD:
    case TYPE_GET_1_0:
        if (m_pFile)
        {
            *pStatusLineLength = sizeof(Ok) - 1;
            return Ok;
        }
        else
        {
            *pStatusLineLength = sizeof(NotFound) - 1;
            return NotFound;
        }
        break;
    case TYPE_GET_0_9:
        // Shouldn't get called for such a request.
        assert ( TYPE_GET_0_9 != m_RequestType );
        return NULL;
    case TYPE_BAD_REQUEST:
        *pStatusLineLength = sizeof(BadRequest) - 1;
        return BadRequest;
    default:
        // This server has no default handling for any other type of request.
        *pStatusLineLength = sizeof(NotImplemented) - 1;
        return NotImplemented;
    }
}

const char* CHTTPContext::GetResponseHeaders(size_t* pResponseHeaderLength)
{
    // Additional error text will be returned as HTML text.
    static const char ContentTypeHTML[] = "Content-type: text/html\r\n";
    static const char NotImplementedHTML[] = "<html>"
                                             "<body><h2>501: Not Implemented.</h2></body>"
                                             "</html>\r\n";
    static const char BadRequestHTML[] = "<html>"
                                         "<body><h2>400: Bad Request.</h2></body>"
                                         "</html>\r\n";
 
    switch (m_RequestType)
    {
    case TYPE_HEAD:
    case TYPE_GET_1_0:
        *pResponseHeaderLength = m_ContentTypeLength;
        return m_pContentType;
    case TYPE_GET_0_9:
        // Shouldn't get called for such a request.
        assert ( TYPE_GET_0_9 != m_RequestType );
        return NULL;
    case TYPE_BAD_REQUEST:
        // Will send error back as HTML text.
        m_pErrorHTML = BadRequestHTML;
        m_ErrorHTMLLength = sizeof(BadRequestHTML) - 1;
        *pResponseHeaderLength = sizeof(ContentTypeHTML) - 1;
        return ContentTypeHTML;
    default:
        // This server has no default handling for any other type of request.
        m_pErrorHTML = NotImplementedHTML;
        m_ErrorHTMLLength = sizeof(NotImplementedHTML) - 1;
        *pResponseHeaderLength = sizeof(ContentTypeHTML) - 1;
        return ContentTypeHTML;
    }
}

int CHTTPContext::BeginResponseContent()
{
    // Have response content to send back so return 1;
    assert ( m_pFile || m_pErrorHTML );
    return 1;
}

size_t CHTTPContext::ReadResponseContentBlock(char*  pBuffer,
                                              size_t BytesToRead)
{
    if (m_pFile)
    {
        // Read content from file.
        return fread(pBuffer, 1, BytesToRead, m_pFile);
    }
    else
    {
        // Read content from HTML text for error.
        size_t BytesLeft = m_ErrorHTMLLength - m_ErrorHTMLOffset;
        
        if (BytesToRead > BytesLeft)
        {
            BytesToRead = BytesLeft;
        }
        memcpy(pBuffer, &m_pErrorHTML[m_ErrorHTMLOffset], BytesToRead);
        m_ErrorHTMLOffset += BytesToRead;

        return BytesToRead;
    }
}

void CHTTPContext::EndResponseContent()
{
    if (m_pFile)
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }
}

void CHTTPContext::Release()
{
    // Just make sure that any files are closed.
    EndResponseContent();

    return;
}


/* Called by lwIP whenever data is sent to an active connection.  The main task
   of this callback is to acknowledge application receipt of the data, parse 
   the received data to determine which file is being requested by the client,
   open the requested file, and start sending it back to the remote client. 
   
   Parameters:
    pvArg is the value set on the active PCB through the call to tcp_arg in
        HTTPAccept() which is a pointer to the HTTP context for this
        connection.
    pPCB is a pointer to the PCB that has just received some data from the 
        client.
    pBuf is a pointer to a PBUF which contains a linked list of the data
        received from the remote client.
    err is passed in from the lwIP stack to indicate if an error occurred.
    
   Returns:
    ERR_ABRT if we end up aborting the connection and ERR_OK otherwise.
*/
err_t CHTTPContext::HTTPRecv(void* pvArg, tcp_pcb* pPCB, pbuf* pBuf, err_t err)
{
    CHTTPContext* pHTTPContext = (CHTTPContext*)pvArg;
    
    // Validate parameters.
    assert ( pPCB );
    assert ( ERR_OK == err );
    
    // If the client has closed their end then pBuf will be NULL so the server
    // should shutdown its end as well.
    if (!pBuf)
    {
        TRACE("HTTP: Closing as client closed its end for connection: ");
        NetworkPrintRemoteAddress(pPCB);
        TRACE("\r\n");
        
        pHTTPContext->CloseConnection();
        pHTTPContext->FreeContext(TRUE);

        return ERR_OK;
    }
        
    // Let lwIP know that the HTTP application has received the data.
    tcp_recved(pPCB, pBuf->tot_len);

    // Just return if we have already freed the client context from a close
    // attempt.
    if (!pvArg)
    {
        TRACE("HTTP: Ignoring HTTPRecv() as client context was already freed for: ");
        NetworkPrintRemoteAddress(pPCB);
        TRACE("\r\n");
        goto Error;
    }

    // The PCB shouldn't change out underneath this object!
    assert ( pPCB == pHTTPContext->m_pPCB );
    
    pHTTPContext->ProcessRequestData(pBuf);

Error:
    // Release the pbuf as the application no longer requires any of its data
    pbuf_free(pBuf);
    
    return ERR_OK;
}


/* Called by lwIP whenever sent data is acknowledged by the remote machine.  
   The main task of this callback is to update the byte count of unacknowledged
   bytes and send more data once the previously sent data has been acknowledged.
   
   Parameters:
    pvArg is the value set on the active PCB through the call to tcp_arg in
        HTTPAccept() which is a pointer to the HTTP context for this
        connection.
    pPCB is a pointer to the PCB that has just received acknowledgement from
        the remote client of sent data.
    AcknowledgeCount is the number of bytes that the remote client has
        acknowledged receipt of.
    
   Returns:
    ERR_ABRT if we end up aborting the connection and ERR_OK otherwise.
*/
err_t CHTTPContext::HTTPSent(void* pvArg, tcp_pcb* pPCB, u16_t AcknowledgeCount)
{
    CHTTPContext* pHTTPContext = (CHTTPContext*)pvArg;

    // Validate parameters.
    assert ( pvArg );
    assert ( pPCB );
    assert ( AcknowledgeCount );

    // Loop through all of the buffers in the sent state to update their
    // unacknowledged count since this acknowledgment can span buffers.
    while (AcknowledgeCount > 0)
    {
        SBuffer*      pSentBuffer = pHTTPContext->m_BufferQueue[pHTTPContext->m_BufferQueueTail];
        
        // We should have a buffer in the sent state if we are getting back
        // acknowledgments from the remote machine.
        assert ( pSentBuffer );
        
        // Update outstanding byte count for this buffer.
        if (AcknowledgeCount >= pSentBuffer->UnacknowledgedBytes)
        {
            // This callback acknowledges all of the data in the buffer at the
            // tail of the queue and possibly spans into the next buffer as
            // well.  It is also possible that there is still more data to be
            // written from this buffer.
            AcknowledgeCount -= pSentBuffer->UnacknowledgedBytes;
            pSentBuffer->UnacknowledgedBytes = 0;
        
            if (0 == pSentBuffer->BytesToWrite)
            {
                // Free the buffer which might cause another client context to be
                // invoked if it was waiting for a buffer.
                pHTTPContext->m_BufferQueue[pHTTPContext->m_BufferQueueTail] = NULL;
                pHTTPContext->m_BufferQueueTail = (pHTTPContext->m_BufferQueueTail + 1) & 1;
                pHTTPContext->m_pHTTPServer->FreeBuffer(pSentBuffer);
                
                // Check to see if this context is in the closing state and all sent
                // buffers are now free.
                if (pHTTPContext->m_ClosingConnection && 
                    !pHTTPContext->m_BufferQueue[pHTTPContext->m_BufferQueueTail])
                {
                    // Free the context and return immediately since the context is
                    // no longer valid.
                    assert ( !pHTTPContext->m_BufferQueue[0] );
                    assert ( !pHTTPContext->m_BufferQueue[1] );
                    assert ( !pHTTPContext->m_WaitingForBuffer );
                    
                    pHTTPContext->FreeContext(FALSE);
                    return ERR_OK;
                }
            }
            else
            {
                // We still have data to write from this buffer so there should
                // no more bytes to acknowledge in another buffer.
                assert ( 0 == AcknowledgeCount );
            }
        }
        else
        {
            // This callback is just acknowledging a portion of the data in
            // this buffer.
            pSentBuffer->UnacknowledgedBytes -= AcknowledgeCount;
            AcknowledgeCount = 0;
        }
    }
    
    // Attempt to send more response data to this client.
    pHTTPContext->SendResponse();
    
    return ERR_OK;
}


/* Called by lwIP every 2 seconds (4 iterations of the tcp_slowtmr) as 
  specified in the tcp_poll() call in HTTPInit().  Can be used to retry
  sending of response data or closing of the PCB.
      
   Parameters:
    pvArg is the value set on the active PCB through the call to tcp_arg in
        HTTPAccept() which is a pointer to the HTTP context for this
        connection.  It will be NULL if we have previously failed to close
        the PCB.
    pPCB is a pointer to the low level lwIP API socket object for this
        connection.
    
   Returns:
    ERR_ABRT if we end up aborting the connection and ERR_OK otherwise.
*/
err_t CHTTPContext::HTTPPoll(void* pvArg, tcp_pcb* pPCB)
{
    CHTTPContext* pHTTPContext = (CHTTPContext*)pvArg;
    
    // Validate parameters.
    assert ( pPCB );
    
    // Check to see if we need to retry closing the PCB.
    if (!pHTTPContext)
    {
        err_t CloseResult;

        TRACE("HTTP: Retrying close for connection to: ");
        NetworkPrintRemoteAddress(pPCB);
        TRACE("\r\n");

        CloseResult = tcp_close(pPCB);
        if (ERR_OK != CloseResult)
        {
            // This is the second failed attempt to close the PCB, force an
            // abort.
            tcp_abort(pPCB);
            return ERR_ABRT;
        }
        else
        {
            return ERR_OK;
        }
    }
    
    // Close the connection if the client hasn't sent its request within the polling interval.
    if (STATE_DONE != pHTTPContext->m_ParseState)
    {
        TRACE("HTTP: Closing connection due to no HTTP request from connection to: ");
        NetworkPrintRemoteAddress(pPCB);
        TRACE("\r\n");
        
        pHTTPContext->CloseConnection();
        pHTTPContext->FreeContext(TRUE);
        
        return ERR_OK;
    }

    // If we have unacknowledged bytes outstanding then we will still
    // receive a HTTPSent() callback from lwIP so no need to retry data sends
    // from here.
    // NOTE: This version will wait forever for a request from the client.
    if (!pHTTPContext->m_BufferQueue[pHTTPContext->m_BufferQueueTail] && 
         pHTTPContext->m_BufferQueue[pHTTPContext->m_BufferQueueHead])
    {
        if (pHTTPContext->m_RetryCount++ > 8)
        {
            // We have already retried sending the data 8 times, so close
            // the connection.
            TRACE("HTTP: Failed multiple retries to send data for connection to: ");
            NetworkPrintRemoteAddress(pPCB);
            TRACE("\r\n");

            pHTTPContext->CloseConnection();
        }
        else
        {
            // Retry sending the response data again.
            TRACE("HTTP: Retrying to send data for connection to: ");
            NetworkPrintRemoteAddress(pPCB);
            TRACE("\r\n");
            
            pHTTPContext->SendResponse();
        }
    }

    return ERR_OK;
}


/* Called by lwIP whenever the PCB is closed or reset by the remote client.
   Gives the application the opportunity to free up the context object.
      
   Parameters:
    pvArg is the value set on the active PCB through the call to tcp_arg in
        HTTPAccept() which is a pointer to the HTTP context for this
        connection.
    Error is the error that caused the connection to be shutdown.  It can be
        set to ERR_ABRT or ERR_RST.
    
   Returns:
    Nothing.
*/
void CHTTPContext::HTTPError(void* pvArg, err_t Error)
{
    CHTTPContext* pHTTPContext = (CHTTPContext*)pvArg;
    
    // Unused parameters.
    (void)Error;
    
    TRACE("HTTP: Forcing connection context to be freed.\r\n");
    
    // Free the context if it is non-NULL.
    if (pHTTPContext)
    {
        pHTTPContext->FreeContext(TRUE);
    }
}


/* Attempts to close the PCB connection.  The context is placed into a closing
   state where it waits for the data written to be acknowledged by the remote
   machine before freeing up its memory.
   
   Parameters:
    None.
   Returns:
    Nothing.
*/
void CHTTPContext::CloseConnection()
{
    err_t CloseResult = ERR_MEM;
    
    // Validate parameters.
    assert ( m_pPCB );
    
    TRACE("HTTP: Closing connection to: ");
    NetworkPrintRemoteAddress(m_pPCB);
    TRACE("\r\n");
    
    // Flag the context as being in the closing state.
    m_ClosingConnection = 1;
    
    // Have no more data to send so no need for poll callback unless the 
    // tcp_close() below should fail and require retrying.
    tcp_poll(m_pPCB, NULL, 0);

    // Atempt to close the connection.
    CloseResult = tcp_close(m_pPCB);
    if (ERR_OK != CloseResult)
    {
        // Close wasn't successful so re-enable the HTTPPoll() callback.
        tcp_poll(m_pPCB, HTTPPoll, 4);
    }
}


/* Frees up the resources used by the context.
   
   Parameters:
    ForcedFree indicates whether a shutdown is being forced and therefore it is
        OK to free the buffers even though they might indicate they still have
        outstanding data.
        
   Returns:
    Nothing.
*/
void CHTTPContext::FreeContext(int ForcedFree)
{
    size_t i;
    
    if (m_pPCB)
    {
        TRACE("HTTP: Freeing context for connection to: ");
        NetworkPrintRemoteAddress(m_pPCB);
        TRACE("\r\n");
    }
    else
    {
        TRACE("HTTP: Freeing connection due to unexpected close.\r\n");
    }
    
    // Free the HTTP context record and any resources it owned.
    if (m_WaitingForBuffer)
    {
        m_pHTTPServer->RemoveWaitingContext(this);
    }
    for (i = 0 ; i < ARRAYSIZE(m_BufferQueue) ; i++)
    {
        SBuffer* pBuffer = m_BufferQueue[i];
        if (pBuffer)
        {
            if (ForcedFree)
            {
                // 0 out the counts if we are forcing a free.
                pBuffer->BytesToWrite = 0;
                pBuffer->UnacknowledgedBytes = 0;
            }
            
            // Free the buffer.
            m_pHTTPServer->FreeBuffer(m_BufferQueue[i]);
            m_BufferQueue[i] = NULL;
        }
    }
    if (m_pRequestHandlerContext)
    {
        m_pRequestHandlerContext->Release();
        m_pRequestHandlerContext = NULL;
    }

    // Clear the callbacks for the PCB as we are getting ready to close it.
    if (m_pPCB)
    {
        // Keep the polling callback enabled incase the previous tcp_close()
        // attempt failed.
        tcp_arg(m_pPCB, NULL);
        tcp_accept(m_pPCB, NULL);
        tcp_recv(m_pPCB, NULL);
        tcp_sent(m_pPCB, NULL);
        tcp_err(m_pPCB, NULL);
        m_pPCB = NULL;
    }

    delete this;
}


/* Processes the HTTP request data as it comes in from the client.

   Parameters:
    pBuf points to the buffer of inbound TCP/IP data from the connection.
    
   Returns:
    Nothing.
*/
void CHTTPContext::ProcessRequestData(pbuf* pBuf)
{
    // Validate parameters.
    assert ( pBuf );
    
    switch (m_ParseState)
    {
    case STATE_FIRST_LINE:
    case STATE_HEADERS:
        BufferRequestLines(pBuf);
        break;
    case STATE_CONTENT:
        ProcessRequestContent(pBuf);
        break;
    case STATE_DONE:
        // We should have received everything already so discard data and
        // return.
        return;
    }

    if (STATE_DONE == m_ParseState)
    {
        // Let the application interface know that we are done sending it
        // the request content.
        if (m_SendRequestContentToHandler)
        {
            m_pRequestHandlerContext->EndRequestContent();
        }

        // And prepare response headers and content to send back to client.
        PrepareResponse();

        // Attempt to allocate a buffer and fill it in with header and data
        // content.
        ReadContent();

        // Attempt to send first segments of response data.
        SendResponse();
    }
}


/* Buffers up the HTTP requests a line at a time and then processes it once
   a complete line has been buffered.

   Parameters:
    pBuf points to the buffer of inbound TCP/IP data from the connection.
    
   Returns:
    Nothing.
*/
void CHTTPContext::BufferRequestLines(pbuf* pBuf)
{
    // Validate parameters.
    assert ( pBuf );
    
    // Loop through all chunks in the pbuf list and parse its content a line
    // at a time.
    while (pBuf)
    {
        char* pCurr = (char*)pBuf->payload;
        u16_t BytesLeft = pBuf->len;
        
        // Loop through the characters in this chunk.
        while (BytesLeft)
        {
            char CurrChar = *pCurr;
            
            if ('\n' == CurrChar)
            {
                // Have encountered end of line.
                m_LineBuffer[m_LineBufferOffset] = '\0';
                
                ParseRequestLine();

                // Reset pointer to start buffering next line.
                m_LineBufferOffset = 0;
            }
            else if ('\r' != CurrChar &&
                     m_LineBufferOffset < ARRAYSIZE(m_LineBuffer)-1)
            {
                // Copy character into line buffer.  Above conditional protects
                // from buffer overflow on the write and leaves room for NULL
                // terminator.
                m_LineBuffer[m_LineBufferOffset++] = CurrChar;
            }
            
            // Advance to the next character in this chunk.
            pCurr++;
            BytesLeft--;
        }
        
        // Advance to the next chunk in the pbuf list.
        pBuf = pBuf->next;
    }
}


/* Processes the HTTP request content data as it comes in from the client.

   Parameters:
    pBuf points to the buffer of inbound TCP/IP data from the connection.
    
   Returns:
    Nothing.
*/
void CHTTPContext::ProcessRequestContent(pbuf* pBuf)
{
    // Validate parameters.
    assert ( pBuf );

    while (pBuf && m_RequestContentLength > 0);
    {
        size_t BytesToWrite = pBuf->len;
        if (BytesToWrite > m_RequestContentLength)
        {
            // Make sure that we don't send the application interface more
            // content than we told it we would send.
            BytesToWrite = m_RequestContentLength;
        }
        if (m_SendRequestContentToHandler)
        {
            // Only send the application interface the request content if
            // it has indicated that it wants to receive the data.
            m_pRequestHandlerContext->WriteRequestContentBlock(pBuf->payload, BytesToWrite);
        }
        m_RequestContentLength -= BytesToWrite;
        
        // Advance to next chunk in the pbuf list.
        pBuf = pBuf->next;
    }

    if (0 == m_RequestContentLength)
    {
        // Have finished receiving the request content so advance to the next
        // state where response data is sent to the client.
        m_ParseState = STATE_DONE;
    }
}


/* Attempts to parse the current line in m_LineBuffer.

   Parameters:
    None.
    
   Returns:
    Nothing.
*/
void CHTTPContext::ParseRequestLine()
{
    switch(m_ParseState)
    {
    case STATE_FIRST_LINE:
        ParseFirstRequestLine();
        break;
    case STATE_HEADERS:
        ParseHeaderRequestLine();
        break;
    default:
        assert ( FALSE );
        break;
    }
}


/* Attempts to parse the first request line found in m_LineBuffer which should
   contain the request method, URI, etc.

   Parameters:
    None.
    
   Returns:
    Nothing.
*/
void  CHTTPContext::ParseFirstRequestLine()
{
    static const char GetMethod[] = "GET ";
    static const char HeadMethod[] = "HEAD ";
    static const char PostMethod[] = "POST ";
    const char*       pURIStart = NULL;
    const char*       pURISrc = NULL;
    char*             pURIDest = NULL;
    char              UnescapedChar = 0;
    int               CharsToEscape = 0;

    // Advance to next state.
    m_ParseState = STATE_HEADERS;

    // This is the first line which indicates method being requested
    // (ie. GET, HEAD, POST).
    if (0 == strncmp(m_LineBuffer, GetMethod, sizeof(GetMethod)-1))
    {
        // Assume a HTTP/1.0 GET request for now.  Will know later if we
        // need to downgrade it to a 0.9 request when version isn't sent.
        m_RequestType = TYPE_GET_1_0;
        pURISrc = &m_LineBuffer[sizeof(GetMethod)-1];
    }
    else if (0 == strncmp(m_LineBuffer, HeadMethod, sizeof(HeadMethod)-1))
    {
        m_RequestType = TYPE_HEAD;
        pURISrc = &m_LineBuffer[sizeof(HeadMethod)-1];
    }
    else if (0 == strncmp(m_LineBuffer, PostMethod, sizeof(PostMethod)-1))
    {
        m_RequestType = TYPE_POST;
        pURISrc = &m_LineBuffer[sizeof(PostMethod)-1];
    }
    else
    {
        // Don't recognize this method so mark it as bad.
        m_RequestType = TYPE_BAD_REQUEST;
        
        // See if the application recognizes and wants to handle it by sending
        // it the full request line.
        StartBadRequest(m_LineBuffer);
        return;
    }

    // Skip additional whitespace that might precede URI even though there
    // shouldn't be any just to be lenient.
    while (' ' == *pURISrc)
    {
        pURISrc++;
    }
    pURIStart = pURISrc;
    pURIDest = (char*)(void*)pURISrc;
    
    // Save away the URI character by character until whitespace or end
    // of line is encountered.
    while(' '  != *pURISrc &&
          '\0' != *pURISrc)
    {
        char CurrChar = *pURISrc;
        
        // Unescape % HEX HEX portions of the URI
        if ('%' == CurrChar)
        {
            CharsToEscape = 2;
            UnescapedChar = 0;
        }
        else if (CharsToEscape)
        {
            UnescapedChar <<= 4;
            
            if (CurrChar >= 'a' && CurrChar <= 'f')
            {
                CurrChar = (CurrChar - 'a') + 10;
            }
            else if (CurrChar >= 'A' && CurrChar <= 'F')
            {
                CurrChar = (CurrChar - 'A') + 10;
            }
            else if (CurrChar >= '0' && CurrChar <= '9')
            {
                CurrChar = CurrChar - '0';
            }
            else
            {
                // Not a hex digit.
                CurrChar = 0;
            }
            
            UnescapedChar |= (CurrChar & 0xF);
            
            if (0 == --CharsToEscape)
            {
                *pURIDest++ = UnescapedChar;
            }
        }
        else
        {
            *pURIDest++ = CurrChar;
        }
        pURISrc++;
    }
    
    if (' ' != *pURISrc)
    {
        // Protocol string didn't follow URI so must be a HTTP/0.9 request.
        if (TYPE_GET_1_0 == m_RequestType)
        {
            m_RequestType = TYPE_GET_0_9;
            // This will have been the last request line for a v0.9 request so
            // advance to the finished state.
            m_ParseState = STATE_DONE;
        }
        else
        {
            m_RequestType = TYPE_BAD_REQUEST;

            // See if the application recognizes and wants to handle it by sending
            // it the full request line.
            StartBadRequest(m_LineBuffer);
            return;
        }
    }
    *pURIDest = '\0';
    
    StartRequestHandler(pURIStart);
}


/* Attempts to parse the header request lines until a blank request line is
   encountered which indicates that start of the content.

   Parameters:
    None.
    
   Returns:
    Nothing.
*/
void  CHTTPContext::ParseHeaderRequestLine()
{
    static const char ContentLengthName[] = "Content-Length";
    char*             pName = m_LineBuffer;
    char*             pValue = NULL;
    char*             pCurr = m_LineBuffer;
    
    if ('\0' == m_LineBuffer[0])
    {
        // Let the request handler know that we have reached the last header.
        if (m_SendRequestHeadersToHandler)
        {
            m_pRequestHandlerContext->EndRequestHeaders();
        }
        
        if (m_RequestContentLength)
        {
            // Determine if the request handler is interested in any request
            // content data that might follow the headers.
            m_ParseState = STATE_CONTENT;
            m_SendRequestContentToHandler = m_pRequestHandlerContext->BeginRequestContent(m_RequestContentLength);
        }
        else
        {
            // There is no request content from the client so server can start
            // to send its response now.
            m_ParseState = STATE_DONE;
        }
        
        return;
    }

    // Find the colon character which terminates the field name.  A whitespace
    // character at the beginning of a header line indicates a line
    // continuation so handle that as well.
    while (':' != *pCurr &&
           ' ' != *pCurr &&
           '\t' != *pCurr )
    {
        pCurr++;
    }
    *pCurr++ = '\0';
    
    // Skip over whitespace to find start of value.
    while (' ' == *pCurr ||
           '\t' == *pCurr)
    {
        pCurr++;
    }
    pValue = pCurr;
    
    // Check for the Content-Length field which tells the server if it should
    // expect the request to have a content body.
    if (0 == strcasecmp(pName, ContentLengthName))
    {
        m_RequestContentLength = strtoul(pValue, NULL, 10);
    }
    
    // Send the header to request handler context interface if it has requested
    // the server to do so.
    if (m_SendRequestHeadersToHandler)
    {
        m_pRequestHandlerContext->WriteRequestHeader(pName, pValue);
    }
}


/* Kicks off the process of handling client specified request.
   
   Parameters:
    pURI is the name of the URI being requested.
        
   Returns:
    Nothing.
*/
void CHTTPContext::StartRequestHandler(const char* pURI)
{
    // Start request by first letting application provider a handler and if it
    // doesn't use this object's default request handler interface instead.
    switch (m_RequestType)
    {
    case TYPE_GET_0_9:
    case TYPE_GET_1_0:
        StartGetRequest(pURI);
        break;
    case TYPE_HEAD:
        StartHeadRequest(pURI);
        break;
    case TYPE_POST:
        StartPostRequest(pURI);
        break;
    default:
        assert ( FALSE );
        break;
    }
    
    // Should have a request handler now: either from application or this
    // context object.
    assert ( m_pRequestHandlerContext );
    
    // Ask the request handler interface if it wants each request header sent
    // to it.  There will be no header for HTTP/0.9 GET request which skips the
    // header state.
    if (STATE_HEADERS == m_ParseState)
    {
        m_SendRequestHeadersToHandler = m_pRequestHandlerContext->BeginRequestHeaders();
    }
}


/* Handles GET requests by first allowing the application to handler it via a
   register request handler.  If it doesn't want to handle the request then the
   server will attempt to satisfy the request from the file system that was
   registers at bind time.
   
   Parameters:
    pURI is the name of the URI being requested.
        
   Returns:
    Nothing.
*/
void CHTTPContext::StartGetRequest(const char* pURI)
{
    // Load the requested file if possible or the file not found
    // content if requested file was not found.
    TRACE("HTTP: GET '%s' from: ", pURI);
    NetworkPrintRemoteAddress(m_pPCB);
    TRACE("\r\n");
    
    if (m_pHTTPServer->m_pRequestHandler)
    {
        // First let the application attempt to handle the GET request.
        m_pRequestHandlerContext = m_pHTTPServer->m_pRequestHandler->HandleGetRequest(pURI);
    }
    if (!m_pRequestHandlerContext)
    {
        // The application doesn't want to handle the GET request so use the
        // default server behaviour.
        OpenFile(pURI);
        m_pRequestHandlerContext = this;
    }
}


/* Handles HEAD requests by first allowing the application to handler it via a
   register request handler.  If it doesn't want to handle the request then the
   server will attempt to satisfy the request from the file system that was
   registers at bind time.
   
   Parameters:
    pURI is the name of the URI being requested.
        
   Returns:
    Nothing.
*/
void CHTTPContext::StartHeadRequest(const char* pURI)
{
    // Load the requested file if possible or the file not found
    // content if requested file was not found.
    TRACE("HTTP: HEAD '%s' from: ", pURI);
    NetworkPrintRemoteAddress(m_pPCB);
    TRACE("\r\n");
    
    if (m_pHTTPServer->m_pRequestHandler)
    {
        // First let the application attempt to handle the HEAD request.
        m_pRequestHandlerContext = m_pHTTPServer->m_pRequestHandler->HandleHeadRequest(pURI);
    }

    if (!m_pRequestHandlerContext)
    {
        // The application doesn't want to handle the HEAD request so use the
        // default server behaviour.
        OpenFile(pURI);
        m_pRequestHandlerContext = this;
    }
}


/* Handles POST requests by allowing the application to handler it via a
   registered request handler.  If it doesn't want to handle the request then
   the server will treat it as an unimplemented request.
   
   Parameters:
    pURI is the name of the URI being requested.
        
   Returns:
    Nothing.
*/
void CHTTPContext::StartPostRequest(const char* pURI)
{
    // Load the requested file if possible or the file not found
    // content if requested file was not found.
    TRACE("HTTP: POST '%s' from: ", pURI);
    NetworkPrintRemoteAddress(m_pPCB);
    TRACE("\r\n");
    
    if (m_pHTTPServer->m_pRequestHandler)
    {
        // First let the application attempt to handle the POST request.
        m_pRequestHandlerContext = m_pHTTPServer->m_pRequestHandler->HandlePostRequest(pURI);
    }

    if (!m_pRequestHandlerContext)
    {
        // The application doesn't want to handle the POST request.
        m_pRequestHandlerContext = this;
    }
}


/* Handles unknown/bad HTTP client requests by sending back an appropriate
   error response.
   
   Parameters:
    pRequest is the complete request line which wasn't recognized by the server.
        
   Returns:
    Nothing.
*/
void CHTTPContext::StartBadRequest(const char* pRequest)
{
    if (m_pHTTPServer->m_pRequestHandler)
    {
        // First let the application attempt to handle this request instead.
        m_pRequestHandlerContext = m_pHTTPServer->m_pRequestHandler->HandleBadRequest(pRequest);
    }
    if (!m_pRequestHandlerContext)
    {
        // The application doesn't want to handle the bad request so just
        // return an appropriate bad request response.
        m_pRequestHandlerContext = this;
    }

    // Ask the request handler interface if it wants each request header sent
    // to it.
    m_SendRequestHeadersToHandler = m_pRequestHandlerContext->BeginRequestHeaders();
}


/* Called from HTTPRecv() to open the specified file that has been requested by
   the HTTP client.  In addition to opening the specified file, it will also
   fill in the headers to match the content being sent vack.  It also handles
   sending back the correct header if the requested file isn't found.
   
   Parameters:
    pURI is the name of the URI being requested.
        
   Returns:
    Nothing.
*/
void CHTTPContext::OpenFile(const char* pURI)
{
    static const char NotFoundHTML[] = "<html>"
                                       "<body><h2>404: File not found.</h2></body>"
                                       "</html>\r\n";
    // The content types supported by this HTTP server.
    static const char  ContentHTML[] = "Content-type: text/html\r\n";
    static const char  ContentGIF[] = "Content-type: image/gif\r\n";
    static const char  ContentPNG[] = "Content-type: image/png\r\n";
    static const char  ContentJPG[] = "Content-type: image/jpeg\r\n";
    static const char  ContentBMP[] = "Content-type: image/bmp\r\n";
    static const char  ContentICO[] = "Content-type: image/x-icon\r\n";
    static const char  ContentStream[] = "Content-type: application/octet-stream\r\n";
    static const char  ContentJScript[] = "Content-type: application/x-javascript\r\n";
    static const char  ContentCSS[] = "Content-type: text/css\r\n";
    static const char  ContentFlash[] = "Content-type: application/x-shockwave-flash\r\n";
    static const char  ContentXML[] = "Content-type: text/xml\r\n";
    static const char  ContentDefault[] = "Content-type: text/plain\r\n";

    // Table used to map file extensions to above content type headers.
    #define CONTENT_HEADER(X) X, sizeof(X)-1
    static const struct
    {
        // File extension.
        const char*     pExtension;
        // Content header to be used for this file extension type.
        const char*     pContentHeader;
        // Length of pContentHeader string without NULL terminator.
        unsigned int    ContentHeaderLength;
    } ContentMapping[] =
    {
        {".html",  CONTENT_HEADER(ContentHTML)},
        {".htm",   CONTENT_HEADER(ContentHTML)},
        {".gif",   CONTENT_HEADER(ContentGIF)},
        {".png",   CONTENT_HEADER(ContentPNG)},
        {".jpg",   CONTENT_HEADER(ContentJPG)},
        {".bmp",   CONTENT_HEADER(ContentBMP)},
        {".ico",   CONTENT_HEADER(ContentICO)},
        {".class", CONTENT_HEADER(ContentStream)},
        {".cls",   CONTENT_HEADER(ContentStream)},
        {".js",    CONTENT_HEADER(ContentJScript)},
        {".css",   CONTENT_HEADER(ContentCSS)},
        {".swf",   CONTENT_HEADER(ContentFlash)},
        {".xml",   CONTENT_HEADER(ContentXML)}
    };
    // Local variables.
    size_t  i;
    char    LocalFilename[HTTP_MAX_FILENAME+1];
    
    // Attempt to open the file.
    if (0 == strcmp(pURI, "/"))
    {
        // Iterate through the list of default root filenames.
        for (i = 0 ; !m_pFile && i < ARRAYSIZE(g_DefaultFilenames) ; i++)
        {
            snprintf(LocalFilename, ARRAYSIZE(LocalFilename), "/%s%s", m_pHTTPServer->m_pRootPathname, g_DefaultFilenames[i]);
            m_pFile = fopen(LocalFilename, "r");
        }
    }
    else
    {
        // Attempt to open the specified URI.
        snprintf(LocalFilename, ARRAYSIZE(LocalFilename), "/%s%s", m_pHTTPServer->m_pRootPathname, pURI);
        m_pFile = fopen(LocalFilename, "r");
    }
    
    // Setup the content type field based on whether the file was found or not.
    if (!m_pFile)
    {
        // The file open failed so return a 404 error with HTML.
        m_pContentType = ContentHTML;
        m_ContentTypeLength = sizeof(ContentHTML) - 1;
        m_pErrorHTML = NotFoundHTML;
        m_ErrorHTMLLength = sizeof(NotFoundHTML) - 1;
    }
    else
    {
        const char* pExtension;
        
        // Configure the content type header based on filename extension.
        pExtension = strrchr(LocalFilename, '.');
        for (i = 0 ; pExtension && i < ARRAYSIZE(ContentMapping) ; i++)
        {
            if (0 == strcmp(pExtension, ContentMapping[i].pExtension))
            {
                m_pContentType = ContentMapping[i].pContentHeader;
                m_ContentTypeLength = ContentMapping[i].ContentHeaderLength;
                break;
            }
        }
        
        // If the type couldn't be found default to plain.
        if (!m_pContentType)
        {
            m_pContentType = ContentDefault;
            m_ContentTypeLength = sizeof(ContentDefault) - 1;
        }
    }    
}


/* Prepares the headers and content to be sent back to the HTTP client as a
   response to its latest request.  It then starts sending out the first set of
   response packets.
   
   Parameters:
    None.
        
   Returns:
    Nothing.
*/
void CHTTPContext::PrepareResponse()
{
    assert ( STATE_DONE == m_ParseState );

    PrepareResponseHeaders();
    
    if (!m_pRequestHandlerContext->BeginResponseContent())
    {
        // The application has no content to send so release it.
        m_pRequestHandlerContext->Release();
        m_pRequestHandlerContext = NULL;
    }
}


/* Prepares the headers and content to be sent back to the HTTP client as a
   response to its latest request.  It then starts sending out the first set of
   response packets.
   
   Parameters:
    None.
        
   Returns:
    Nothing.
*/
void CHTTPContext::PrepareResponseHeaders()
{
    static const char   BlankLine[] = "\r\n";
    const char*         pStatusLine = NULL;
    const char*         pExtraHeaders = NULL;
    size_t              StatusLineLength = 0;
    size_t              ExtraHeadersLength = 0;
    size_t              i = 0;
    
    if (TYPE_GET_0_9 == m_RequestType)
    {
        // Don't return any headers for a HTTP/0.9 GET request.
        m_HeaderArray[0].pHeaderString = NULL;
        m_HeaderArray[0].HeaderLength = 0;
        return;
    }
    
    // Prepare the response status.
    pStatusLine = m_pRequestHandlerContext->GetStatusLine(&StatusLineLength);
    
    // Application must return a status line.
    assert ( pStatusLine && StatusLineLength > 0 );
    
    m_HeaderArray[0].pHeaderString = pStatusLine;
    m_HeaderArray[0].HeaderLength = StatusLineLength;
    
    // The second header returned will always be the server type, no
    // matter what response we send back.
    m_HeaderArray[1].pHeaderString = m_pHTTPServer->m_ServerHeader;
    m_HeaderArray[1].HeaderLength = m_pHTTPServer->m_ServerHeaderLength;

    // Find out what other headers the application would like to send back to
    // the HTTP client.
    pExtraHeaders = m_pRequestHandlerContext->GetResponseHeaders(&ExtraHeadersLength);
    if (pExtraHeaders)
    {
        m_HeaderArray[2].pHeaderString = pExtraHeaders;
        m_HeaderArray[2].HeaderLength = ExtraHeadersLength;
        i = 3;
    }
    else
    {
        // Won't be using all of the elements of m_HeaderArray.
        i = 2;
    }
    
    // Finally send the blank line header which separates the headers from the
    // entity body.
    m_HeaderArray[i].pHeaderString = BlankLine;
    m_HeaderArray[i].HeaderLength = sizeof(BlankLine) -1;
}

/* Attempts to allocate a SBuffer object for reading the data into.  If such a
   buffer is available then read HTTP headers and file data into the buffer.
   If the buffer can't be allocated, then add this client to a queue to be
   processed later.

   Parameters:
    None.
        
   Returns:
    Nothing.
*/
void CHTTPContext::ReadContent()
{
    // Local variables.
    SBuffer*     pWriteBuffer = m_BufferQueue[m_BufferQueueHead];
    unsigned int BytesLeftInBuffer;
    char*        pData;
    unsigned int i;
    
    
    // Return now if there is already a buffer filled with data in the process
    // of being written out to the connection.  Also return if there is no more
    // data to be sent to this client.
    if (pWriteBuffer ||
        (m_HeaderIndex >= ARRAYSIZE(m_HeaderArray) &&
         !m_pRequestHandlerContext))
    {
        return;
    }

    // Attempt to allocate a new SBuffer.
    pWriteBuffer = m_pHTTPServer->AllocateBuffer(this);
    if (!pWriteBuffer)
    {
        // There is no buffer available at this time.  Will be called back
        // later when one becomes available.
        return;
    }
    assert ( 0 == pWriteBuffer->BytesToWrite );
    assert ( 0 == pWriteBuffer->UnacknowledgedBytes );
    
    // Buffer is free.  Now fill it with headers and file data.
    BytesLeftInBuffer = sizeof(pWriteBuffer->Data);
    pData = pWriteBuffer->Data;
    pWriteBuffer->pWrite = pData;
    
    // Copy remaining headers into free buffer.
    for (i = m_HeaderIndex ; 
         i < ARRAYSIZE(m_HeaderArray) ; 
         i++)
    {
        unsigned int BytesToCopy;
        unsigned int HeaderBytesLeft;
        
        assert ( i == m_HeaderIndex );
        
        if (!m_HeaderArray[i].pHeaderString)
        {
            // Not all (or any) of the headers were used so mark as done and
            // no need to copy any more so break out of loop.
            m_HeaderIndex = ARRAYSIZE(m_HeaderArray);
            break;
        }
        
        // Determine how many bytes are to be copied.
        HeaderBytesLeft = m_HeaderArray[i].HeaderLength - 
                          m_HeaderOffset;
        if (HeaderBytesLeft > BytesLeftInBuffer)
        {
            BytesToCopy = BytesLeftInBuffer;
        }
        else
        {
            BytesToCopy = HeaderBytesLeft;
        }
        
        // Perform the copy.
        memcpy(pData, 
               m_HeaderArray[i].pHeaderString + m_HeaderOffset,
               BytesToCopy);
               
        // Update buffer information.
        pData += BytesToCopy;
        BytesLeftInBuffer -= BytesToCopy;
        HeaderBytesLeft -= BytesToCopy;
        if (0 == HeaderBytesLeft)
        {
            // Advance to next header.
            m_HeaderIndex++;
            m_HeaderOffset = 0;
        }
        else
        {
            // Advance offset within this header and exit since buffer is full.
            assert ( 0 == BytesLeftInBuffer );
            
            m_HeaderOffset += BytesToCopy;
            break;
        }
    }
    
    // Copy file data into buffer for sending.
    if (m_pRequestHandlerContext)
    {
        size_t BytesRead;
        
        BytesRead = m_pRequestHandlerContext->ReadResponseContentBlock(pData, BytesLeftInBuffer);
        if (BytesRead < BytesLeftInBuffer)
        {
            // Must have hit end of file, so close it.
            m_pRequestHandlerContext->EndResponseContent();
            m_pRequestHandlerContext->Release();
            m_pRequestHandlerContext = NULL;
        }
        pData += BytesRead;
        BytesLeftInBuffer -= BytesRead;
    }
    
    // Update the buffer valid byte count.
    pWriteBuffer->BytesToWrite = ARRAYSIZE(pWriteBuffer->Data) - 
                                 BytesLeftInBuffer;
                                 
    // Update the client context to indicate that it now has a buffer ready to
    // be written out to the client connection.
    m_BufferQueue[m_BufferQueueHead] = pWriteBuffer;
}


/* Attempts to send HTTP client data already read into buffer and also reads
   in more data to the buffer for sending as necessary.  This data will be
   composed of HTTP headers and file data.
   
   Parameters:
    None.
    
   Returns:
    Nothing.
*/
void CHTTPContext::SendResponse()
{
    int          ConnectionClosed;

    // Attempt to send any data still in write buffer.
    ConnectionClosed = WriteData();
    
    // Return now if the connection was closed.
    if (ConnectionClosed)
    {
        return;
    }
    
    // Read in more content if possible.
    ReadContent();

    // UNDONE: Once I have a performance test available, try removing this to see if it has any impact on performance.
    //         Also want to wait for DMA based driver.
    // Attempt to send data that was just read into buffer.
    WriteData();
}


/* Sends any remaining response data to the HTTP client and closes the
   connection once the data has been sent.
   
   Parameters:
    None.

   Returns:
    1 if the last chunk was sent and the connection closed and 0 otherwise.
*/
int CHTTPContext::WriteData()
{
    // Local variables.
    err_t    WriteResult = ERR_MEM;
    size_t   BytesToWrite;
    size_t   SendBufferSize;
    SBuffer* pWriteBuffer = m_BufferQueue[m_BufferQueueHead];

    // Just return if there isn't a write buffer with data to send.
    if (!pWriteBuffer)
    {
        return 0;
    }

    // Don't try to send more bytes than what the connection will accept.
    BytesToWrite = pWriteBuffer->BytesToWrite;
    SendBufferSize = tcp_sndbuf(m_pPCB);
    if (BytesToWrite > SendBufferSize)
    {
        BytesToWrite = SendBufferSize;
    }
    
    // Keep attempting to send response data and shrinking the write until it
    // succeeds or it shrinks down to 0 bytes.
    while (BytesToWrite > 0)
    {
        // Attempt to write the response data out to the PCB.
        WriteResult = tcp_write(m_pPCB, 
                                pWriteBuffer->pWrite, 
                                BytesToWrite,
                                0);
        if (ERR_OK == WriteResult)
        {
            // Write was successful so update buffer descriptors in the context
            // objects.
            pWriteBuffer->pWrite += BytesToWrite;
            pWriteBuffer->BytesToWrite -= BytesToWrite;
            pWriteBuffer->UnacknowledgedBytes += BytesToWrite;
            
            // Successfully queued up data for sending so reset retry count.
            m_RetryCount = 0;
            break;
        }
        else if (ERR_MEM == WriteResult)
        {
            // Failed to perform the write due to out of memory error.
            // Check to see if the send queue if full.  If not, half the write
            // size and try again.
            if (0 == tcp_sndqueuelen(m_pPCB))
            {
                break;
            }
            // UNDONE: Rather than half it, subtract the size of a MSS.
            BytesToWrite >>= 1;
        }
        else
        {
            // Received unexpected error.
            break;
        }
    }
    
    if (0 == pWriteBuffer->BytesToWrite)
    {
        // There are no more bytes to write from the buffer so advance the
        // header pointer to the next item in the buffer queue.
        m_BufferQueueHead = (m_BufferQueueHead + 1) & 1;

        // If there are no more bytes to send and the client has finished sending
        // its request then we should attempt to close the connection.
        if (m_HeaderIndex >= ARRAYSIZE(m_HeaderArray) &&
            !m_pRequestHandlerContext &&
            STATE_DONE == m_ParseState)
        {
            TRACE("HTTP: Response send completed to: ");
            NetworkPrintRemoteAddress(m_pPCB);
            TRACE("\r\n");
            CloseConnection();
        
            return 1;
        }
    }
    
    return 0;
}


/* Called for a context which was waiting for a buffer when a buffer is freed
   by another connection.
   
   Parameters:
    None.
    
   Returns:
    Nothing.
*/
void CHTTPContext::BufferAvailable()
{
    // Allow the context to allocate the buffer and fill it in with data.
    ReadContent();
    
    // Allow the context to send the data that was just read into the
    // buffer.
    SendResponse();

    // Tell lwIP to send the segments for this PCB as it normally only
    // queues up segments automatically for the PCB whose callback is
    // currently being called.
    tcp_output(m_pPCB);
}



/* Constructor */
CHTTPServer::CHTTPServer()
{
    m_pHTTPListenPCB = NULL;
    m_pContextWaitingHead = NULL;
    m_pContextWaitingTail = NULL;
    m_pRootPathname = NULL;
    m_pRequestHandler = NULL;
    m_ServerHeaderLength = 0;;
    m_ServerHeader[0] = '\0';
    memset(m_BufferPool, 0, sizeof(m_BufferPool));
}


/* Attach IRequestHandler to the HTTP server to allow application to add
   custom GET/POST handling.

   Parameters:
    pHandler is a pointer to the application specific request handler object
        to be used by the HTTP server when it receives GET/POST requests.

   Returns:
    Returns 0 on successful attachment and a positive value otherwise.
*/
int CHTTPServer::AttachRequestHandler(IHTTPRequestHandler* pHandler)
{
    m_pRequestHandler = pHandler;
    
    return 0;
}


/* Initializes the HTTP server using the raw lwIP APIs.  This consists of
   creating a PCB (low level lwIP socket), binding it to port 80 and then
   listening for connections on this port.  When subsequent connection comes
   into the HTTP server, a callback will be made into the HTTPAccept()
   function that is registered here as well.
   
   Parameters:
    pRootPathame is the pathname of the default root directory from which the
        HTTP GET requests are satisfied.
    pServerName is the name of the server to be returned to the HTTP client
        in the Server header.
    BindPort is the TCP/IP port to which the HTTP server should listen for
        incoming requests.  The default port for HTTP would be 80.
    
   Returns:
    0 on success and a positive error code otherwise.
*/
int CHTTPServer::Bind(const char*    pRootPathname,
                      const char*    pServerName,
                      unsigned short BindPort)
{
    int         Return = 1;
    tcp_pcb*    pcb = NULL;
    int         Length = -1;
    err_t       Result;

    // Validate parameters.
    assert ( pRootPathname && pServerName );
    
    // Create server header string.
    Length = snprintf(m_ServerHeader, ARRAYSIZE(m_ServerHeader), "Server: %s\r\n", pServerName);
    if (Length < 0 || Length >= (int)ARRAYSIZE(m_ServerHeader))
    {
        printf("error: '%s' is too long for the HTTP server name string.\r\n", pServerName);
        goto Error;
    }
    m_ServerHeaderLength = (size_t)Length;
    
    // Save away root directory name.
    m_pRootPathname = pRootPathname;
    
    // Create the PCB (low level lwIP socket) for the HTTP server to listen to
    // on well known HTTP port 80.
    pcb = tcp_new();
    if (!pcb)
    {
        printf("error: Failed to create new connection for HTTP server to listen upon.\r\n");
        goto Error;
    }
    
    // Bind the PCB to port 80.
    Result = tcp_bind(pcb, IP_ADDR_ANY, BindPort);
    if (ERR_OK != Result)
    {
        printf("error: HTTP server failed to bind to port %d.\r\n", BindPort);
        goto Error;
    }
    
    // Listen on this PCB.
    m_pHTTPListenPCB = tcp_listen(pcb);
    if (!m_pHTTPListenPCB)
    {
        printf("error: HTTP server failed to listen on port %d.\r\n", BindPort);
        goto Error;
    }
    
    // pcb was freed by successful tcp_listen() call.
    pcb = NULL;
    
    // When connections come in on port 80, we want lwIP to call our
    // HTTPAccept() function to handle the connection.
    tcp_arg(m_pHTTPListenPCB, this);
    tcp_accept(m_pHTTPListenPCB, HTTPAccept);
    
    // Return to the main program since there is nothing left to do until a
    // client tries to make a connection on port 80, at which time lwIP will
    // get the connection packet from the Ethernet driver and make the callback
    // into HTTPAccept().
    
    Return = 0;
Error:
    if (pcb)
    {
        tcp_close(pcb);
    }
    return Return;
}


/* Called by lwIP whenever a connection is made on the PCB which is listening
   on port 80 for HTTP requests.  The main task of this callback is to create
   any necessary state to track this unique HTTP client connection and then
   setup the callbacks to be called when activity occurs on this client
   connection. 
   
   Parameters:
    pvArg is the value set on the listening PCB through the call to tcp_arg in
        HTTPInit() which is a pointer to the intitialized network object.
    pNewPCB is a pointer to the new PCB that was just allocated by the lwIP
        stack for this new client connection on port 80.
    err is passed in from the lwIP stack to indicate if an error occurred.
    
   Returns:
    ERR_MEM if we don't have enough memory for this connection and ERR_OK 
    otherwise.
*/
err_t CHTTPServer::HTTPAccept(void* pvArg, tcp_pcb* pNewPCB, err_t err)
{
    CHTTPServer*    pHTTPServer = (CHTTPServer*)pvArg;
    CHTTPContext*   pHTTPContext = NULL;
    (void)err;
    
    // Validate parameters.
    assert ( pvArg );
    assert ( pNewPCB );
    
    // Let user know that we have just accepted a new connection.
    TRACE("HTTP: Accepting client connection from: ");
    NetworkPrintRemoteAddress(pNewPCB);
    TRACE("\r\n");
    
    // Let the lwIP stack know that the HTTP application has successfuly
    // received the client connection on the listening PCB.
    tcp_accepted(pHTTPServer->m_pHTTPListenPCB);

    // UNDONE: Remove this new call and use a pool instead.
    // Allocate memory to track context for this client connection and
    // associate with the PCB.
    pHTTPContext = new CHTTPContext(pHTTPServer, pNewPCB);
    if (!pHTTPContext)
    {
        TRACE("HTTP: Failed to allocate client context for: ");
        NetworkPrintRemoteAddress(pNewPCB);
        TRACE("\r\n");
        
        // lwIP will shutdown this connection when it get back a return value
        // other than ERR_OK.
        return ERR_MEM;
    }
    
    return ERR_OK;
}


/* Removes the specified context from the linked list of waiting contexts.
   
   Parameters:
    pHTTPContext is a pointer to the context object for the client connection
        to be removed.
        
   Returns:
    Nothing.
*/
void CHTTPServer::RemoveWaitingContext(CHTTPContext* pHTTPContext)
{
    // Find this node in the waiting queue and remove it.
    CHTTPContext*  pPrev = NULL;
    CHTTPContext*  pCurr = m_pContextWaitingHead;
    while(pCurr)
    {
        CHTTPContext* pNext = pCurr->GetNext();

        if (pCurr == pHTTPContext)
        {
            // Found this context.  Now remove it and exit loop.
            pCurr->RemovedFromWaitingList(pPrev);
            if (!pPrev)
            {
                // This node was at the head so update the head pointer.
                m_pContextWaitingHead = pNext;
            }
            if (!pNext)
            {
                // This node was at the tail so reset the tail to previous node.
                m_pContextWaitingTail = pPrev;
            }
            break;
        }
        pPrev = pCurr;
        pCurr = pNext;
    }
    // We should always find this context in the list.
    assert ( pCurr == pHTTPContext );
}


/* Attempts to allocate a SBuffer object into which content can be placed for
   sending back to the HTTP client.  If it fails to find a free buffer in the
   server's pool then it will add the client context to a queue and return
   NULL.
   
   Parameters:
    pHTTPContext is a pointer to the context asking for a buffer object.
    
   Returns:
    A pointer to a free SBuffer object or NULL if none are free.
*/
SBuffer* CHTTPServer::AllocateBuffer(CHTTPContext* pHTTPContext)
{
    CHTTPContext* pPrev = NULL;
    
    // Validate parameters.
    assert ( pHTTPContext );
    
    // Local variables.
    size_t  i;
    
    // Just return NULL if this context is already waiting for a buffer.
    if (pHTTPContext->IsWaitingForBuffer())
    {
        return NULL;
    }
    
    // Search for a free buffer in the pool.
    for (i = 0 ; i < ARRAYSIZE(m_BufferPool) ; i++)
    {
        // A buffer is free if it has no bytes left to write to the connection
        // and the remote machine has acknowledged all previously written bytes
        // from this buffer.
        SBuffer* pBuffer = &(m_BufferPool[i]);
        
        if (0 == pBuffer->BytesToWrite &&
            0 == pBuffer->UnacknowledgedBytes)
        {
            return pBuffer;
        }
    }
    
    // Get here if there wasn't a free buffer in the pool so add this context
    // to the queue to be serviced later.
    assert (!pHTTPContext->GetNext());
    if (m_pContextWaitingTail)
    {
        // Add to end of list.
        pPrev = m_pContextWaitingTail;
        m_pContextWaitingTail = pHTTPContext;
    }
    else
    {
        // Adding entry to empty queue.
        assert (!m_pContextWaitingHead);
        
        m_pContextWaitingHead = pHTTPContext;
        m_pContextWaitingTail = pHTTPContext;
    }
    pHTTPContext->AddToWaitingList(pPrev);
    
    // Let calling context know that it will have to wait until later for a
    // buffer.
    return NULL;
}


/* Frees a SBuffer object back into the server's pool.  If there is a client
   context waiting in the queue, then callback into the context and let
   it use the buffer now that it is free.
   
   Parameters:
    pBuffer is a pointer to the buffer to be freed.
    
   Returns:
    Nothing.
*/
void CHTTPServer::FreeBuffer(SBuffer* pBuffer)
{
    // Validate parameters.
    assert ( pBuffer );
    
    // The SBuffer shouldn't be in use when this routine is called.
    assert ( 0 == pBuffer->BytesToWrite );
    assert ( 0 == pBuffer->UnacknowledgedBytes );
    
    // See if there are other client connection waiting for a buffer to become
    // free.  If so, give it an apportunity to make use of this one.
    if (m_pContextWaitingHead)
    {
        CHTTPContext* pHTTPContext = m_pContextWaitingHead;
        
        // Remove the context from the waiting list.
        m_pContextWaitingHead = pHTTPContext->GetNext();
        if (!m_pContextWaitingHead)
        {
            // We just emptied the list so clear tail pointer as well.
            assert ( m_pContextWaitingTail == pHTTPContext );

            m_pContextWaitingTail = NULL;
        }
        
        // Let context know that it is no longer in a list.
        pHTTPContext->RemovedFromWaitingList(NULL);
        pHTTPContext->BufferAvailable();
    }
}


/* Prints out the remote machine IP address and port number.

   Parameters:
    pPCB is a pointer to the TCP protocol control block.
    
   Returns:
    Nothing.
*/
static void NetworkPrintRemoteAddress(tcp_pcb* pPCB)
{
    // Validate paramters.
    assert ( pPCB );
    
    if (HTTP_TRACE)
    {
        SNetwork_PrintAddress(&pPCB->remote_ip);
        printf(":%u", pPCB->remote_port);
    }
}
