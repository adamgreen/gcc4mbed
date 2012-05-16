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
/* Header file for HTTP Server functionality. */
#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_

#include <mbed.h>
#include "lwip/tcp.h"



// UNDONE: Make protected member of CHTTPServer object?
/** Structure used to buffer data being read in from content files and then
    written out the TCP/IP connections.
*/
struct SBuffer
{
    char*           pWrite;                 /**< Pointer into Data where the next tcp_write() should begin.  There are BytesToWrite valid bytes left in the Data array to be sent. */
    // UNDONE: These can be shorts since mbed only has a total of 64k anyway.
    unsigned int    BytesToWrite;           /**< Number of bytes in buffer left to be written to outbound connection. */
    unsigned int    UnacknowledgedBytes;    /**< Number of bytes outstanding on network waiting for acknowledgment. */
    char            Data[TCP_SND_BUF];      /**< The actual buffer holding data to be sent to remote client. */
};





// *****************
// * C++ Interface *
// *****************
/** Interface to be returned from an application to provide callbacks for a
    specific HTTP request.
*/
class IHTTPRequestHandlerContext
{
public:
    /** Called by the CHTTPServer object before any WriteRequestHeader() calls
        to let the application to know to expect subsequent 
        WriteRequestHeader() calls.
        
        @returns 0 if the application isn't interested in receiving subsequent
                 WriteRequestHeader() and EndRequestHeaders() callbacks.
                 Returns non-zero value to receive headers.
    */
    virtual int  BeginRequestHeaders() = 0;

    /** Called by CHTTPServer for each HTTP request header sent by the HTTP 
        client.  The server won't issue this callback if 0 was previously
        returned by the application from the BeginRequestHeaders() method.
        
        @param pHeaderName is the name of this header field.  If this string
               is empty then this header line is a continuation of the previous
               header line which did have a field name.  The string
               pointed to by this parameter has a short lifetime and is only
               valid during the duration of this call.
        @param pHeaderValue is the string value of this header field.  The
               string pointed to by this parameter has a short lifetime and is
               only valid during the duration of this call.
    */
    virtual void WriteRequestHeader(const char* pHeaderName, 
                                    const char* pHeaderValue) = 0;
    
    /** Called by the CHTTPServer object after it has completed all of the
        necessary WriteRequestHeader() calls to enumerate the headers of the
        HTTP client's request.
    */
    virtual void EndRequestHeaders() = 0;
    

    /** Called by CHTTPServer to let the application know that it should expect
        subsequent WriteRequestContentBlock() calls for processing of the
        request content as it arrives on the TCP/IP socket.
        
        Only POST requests from a client should include such content data.
        
        @param ContentSize is the size of the request content to be sent from
               the HTTP client.  This value indicates the total number of bytes
               that should result from all of the WriteRequestContentBlock()
               calls to follow.
               
        @returns 0 if the application isn't interested in receiving subsequent
                 WriteRequestContentBlock() and EndRequestContent() callbacks.
                 Returns non-zero value to receive request content data.
    */
    virtual int BeginRequestContent(size_t ContentSize) = 0;
    
    /** Called for each block of request data received from the HTTP client.
        The server won't issue this callback if the application returned 0 from
        the BeginRequestContent() method.
        
        @param pBlockBuffer is a pointer to the block of request content data
               most recently received from the HTTP client.
        @param BlockBufferSize is the number of bytes contained in 
               pBlockBuffer.
    */
    virtual void WriteRequestContentBlock(const void* pBlockBuffer, 
                                          size_t BlockBufferSize) = 0;
                                  
    /** Called by CHTTPServer after it has completed all of the 
        WriteRequestContentBlock() calls for processing of the request content
        data sent from the HTTP client.
    */
    virtual void EndRequestContent() = 0;
    
    
    /** Called by CHTTPServer object to get the status line from the
        application to be returned to the HTTP client for this request.
        Examples: "HTTP/1.0 200 OK\r\n" -or- "HTTP/1.0 404 File no found\r\n"
        
        @param pStatusLineLength points to the length to be filled in by the
               application indicating the length, in character, of the returned
               status line.  It should not include the NULL terminator.  It
               must have a non-zero value.
               
        @returns a pointer to the status line to be returned to the HTTP client
                 for this request.  It should start with the "HTTP/1.0" string
                 and end with a newline designator of "\r\n".  The return value
                 can't be NULL.
    */
    virtual const char* GetStatusLine(size_t* pStatusLineLength) = 0;
    
    /** Called by the CHTTPServer object to get all of the response headers
        that the application would like to be returned to the HTTP client.  The
        CHTTPServer will append this header string to the 
        "Server: ServerName\r\n" header that it always sends.
        
        Example: "Content-Length: 100\r\n"
        
        @param pResponseHeaderLength points to the length to be filled in by
               the application indicating the length, in character, of the
               returned header string.  It should not include the NULL
               terminator and can be 0 if the returned value is NULL.
               
        @returns a pointer to the extra headers to be returned to the HTTP
                 client for this request.  It can be NULL if the application
                 has no special headers to be returned.
    */
    virtual const char* GetResponseHeaders(size_t* pResponseHeaderLength) = 0;
    
    /** Called by CHTTPServer to let application know that there will be
        subsequent ReadResponseContentBlock() calls to obtain the content data
        from the application to be sent back to the HTTP client in the
        response.
        
        @returns 0 if the application has no content to be sent back in the
                 HTTP response.
    */
    virtual int BeginResponseContent() = 0;
    
    /** Called by CHTTPServer to get the next block of response data to be
        sent back to the HTTP client.
        
        @param pBuffer is a pointer to the buffer to be filled in by the
               application with data to be sent back to the HTTP client in the
               response.
        @param BytesToRead indicates the number of bytes that should be placed
               into pBuffer by the application.  The application should only
               place fewer than this requested amount on the last block of
               data to be sent back to the HTTP client.
        
        @returns The number of bytes that were actually placed in pBuffer for
                 this call.  It can only be smaller than BytesToRead on the
                 last block to be sent since the server uses such truncated
                reads to know when it has sent the last block of response
                data.
    */
    virtual size_t ReadResponseContentBlock(char*  pBuffer,
                                            size_t BytesToRead) = 0;
    
    
    /** Called by CHTTPServer to let the application know that it has finished
       sending all of the response data to the HTTP client.  The application
       can use such a call to close any files that it has open for satsifying
       this request.
    */
    virtual void EndResponseContent() = 0;
    
    /** Called by the CHTTPServer object when it is completely done processing
       this HTTP response request so that the application can free up any
       resources it has associated with this HTTP client request, including the
       IHTTPRequestHandlerContext derived object servicing these callbacks.
    */
    virtual void Release() = 0;
};


/** The application can provide an IRequestHandler interface implementation to
 *  the HTTP server object.  The methods on such attached interface objects will
 *  be called by the server before it attempts default handling of HTTP
 *  requests.  This gives the application an opportunity to provide overrides
 *  for handling POST requests or GET requests in a certain part of the URI
 *  namespace.
 */
class IHTTPRequestHandler
{
public:
    /** Called by the server for each GET request.  If the application would
        like to handle the GET request rather than using the default server
        code then the application should return a valid 
        IHTTPRequestHandlerContext object pointer on which the server will call
        methods.  These method calls will inform the application of which
        headers were sent in the request and then obtain the response data to
        be sent back to the HTTP client.  Returning a NULL pointer indicates 
        that the server should search in its pRootPathname directory for the
        requested file.
        
        @param pURI is a pointer to the URI being requested by the HTTP
               client.  This is a short lived object that will only be
               valid during this call.
               
        @returns NULL if the CHTTPServer object should process the GET
                 request on its own.  Otherwise return an
                 IHTTPRequestHandlerContext object pointer so that the
                 application can be notified of additional request
                 information and provide the response data.
    */
    virtual IHTTPRequestHandlerContext* HandleGetRequest(const char* pURI) = 0;
    
    /** Called by the server for each HEAD request.  If the application would
        like to handle the HEAD request rather than using the default server
        code then the application should return a valid 
        IHTTPRequestHandlerContext object pointer on which the server will call
        methods.  These method calls will inform the application of which
        headers were sent in the request and then obtain the response data to
        be sent back to the HTTP client.  Returning a NULL pointer indicates 
        that the server should search in its pRootPathname directory for the
        requested file.
        
        @param pURI is a pointer to the URI being requested by the HTTP
               client.  This is a short lived object that will only be
               valid during this call.
               
        @returns NULL if the CHTTPServer object should process the HEAD
                 request on its own.  Otherwise return an
                 IHTTPRequestHandlerContext object pointer so that the
                 application can be notified of additional request
                 information.
    */
    virtual IHTTPRequestHandlerContext* HandleHeadRequest(const char* pURI) = 0;
    
    /** Called by the server for each POST request.  If the application would
        like to handle the POST request then it should return a valid 
        IHTTPRequestHandlerContext object pointer on which the server will call
        methods.  These method calls will inform the application of which
        headers were sent in the request and then obtain the response data to
        be sent back to the HTTP client.  Returning a NULL pointer indicates 
        that the server should fail this POST request with a not implemented
        error.
        
        @param pURI is a pointer to the URI being requested by the HTTP
               client.  This is a short lived object that will only be
               valid during this call.
               
        @returns NULL if the CHTTPServer object should fail the POST
                 request.  Otherwise return an IHTTPRequestHandlerContext 
                 object pointer so that the application can be notified of
                 additional request information and provide the response data.
    */
    virtual IHTTPRequestHandlerContext* HandlePostRequest(const char* pURI) = 0;
    
    /** Called by the server for each unrecognized request.  If the application
        would to handle such request then it should return a valid 
        IHTTPRequestHandlerContext object pointer on which the server will call
        methods.  These method calls will inform the application of which
        headers were sent in the request and then obtain the response data to
        be sent back to the HTTP client.  Returning a NULL pointer indicates 
        that the server should fail this bad request with an appropriate error.
        
        @param pRequest is a pointer to the request being made by the HTTP
               client.  This is a short lived object that will only be
               valid during this call.
               
        @returns NULL if the CHTTPServer object should fail this
                 request.  Otherwise return an IHTTPRequestHandlerContext 
                 object pointer so that the application can be notified of
                 additional request information and provide the response data.
    */
    virtual IHTTPRequestHandlerContext* HandleBadRequest(const char* pRequest) = 0;
};


// Forward declaration of class used to hold context for each HTTP client 
// connection.
class CHTTPContext;


/** HTTP server class.
*/
class CHTTPServer
{
public:
    /** Constructor */
    CHTTPServer();
    
    /** Attach IRequestHandler to the HTTP server to allow application to add
        custom GET/HEAD/POST handling.
    
        @param pHandler is a pointer to the application specific request handler object
               to be used by the HTTP server when it receives GET/POST requests.
        
        @returns 0 on successful attachment and a positive value otherwise.
    */
    int AttachRequestHandler(IHTTPRequestHandler* pHandler);
    
    /** Initializes the HTTP server object by binding it to to the specified
        port number.

        @param pRootPathame is the pathname of the default root directory from which the
               HTTP GET requests are satisfied.
        @param pServerName is the name of the server to be returned to the HTTP client
               in the Server header.
        @param BindPort is the TCP/IP port to which the HTTP server should listen for
                incoming requests.  The default port for HTTP would be 80.
    
        @returns 0 on success and a positive value otherwise.
    */
    int Bind(const char*    pRootPathname,
             const char*    pServerName = "lwIP_HTTPServer/1.0",
             unsigned short BindPort = 80);

protected:
    // Static Method Prototypes for lwIP Callbacks.
    static err_t        HTTPAccept(void* pvArg, tcp_pcb* pNewPCB, err_t err);

    // Methods called from CHTTPContext.
    friend class CHTTPContext;
    void                FreeBuffer(SBuffer* pBuffer);
    void                RemoveWaitingContext(CHTTPContext* pHTTPContext);
    SBuffer*            AllocateBuffer(CHTTPContext* pHTTPContext);

    // Listening port for HTTP Server.
    tcp_pcb*                m_pHTTPListenPCB;
    // Head and tail pointers for queue of client contexts that are waiting to
    // be allocated a SBuffer from the BufferPool.
    CHTTPContext*           m_pContextWaitingHead;
    CHTTPContext*           m_pContextWaitingTail;
    // The default root directory from which the HTTP GET requests are satisfied.
    const char*             m_pRootPathname;
    // Pointer to interface from application allowing it to handle HTTP requests.
    IHTTPRequestHandler*    m_pRequestHandler;
    // Server header returned to HTTP clients.
    size_t                  m_ServerHeaderLength;
    char                    m_ServerHeader[64];
    // Pool of buffers to use for transitioning data from the filesystem to
    // the network.
    SBuffer                 m_BufferPool[HTTP_BUFFER_POOL];
};


#endif /* HTTPSERVER_H_ */
