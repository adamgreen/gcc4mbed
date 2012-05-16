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
/* The purpose of this sample program is to provide an example of how to
   create a simple HTTP server using the raw callback based APIs provided by
   the lwIP network stack.
   
   It implements a very simple HTTP server that is able to source 
   files from the FLASH on the LPC1768 processor.
*/
#include <mbed.h>
#include "FakeFileSystem.h"
#include "SDFileSystem.h"
#include "network.h"
#include "HTTPServer.h"
#include "debug.h"
#include "lwip/stats.h"


// Name of this sample
#define APPLICATION_NAME "HTTPServer"

// Set to appropriate "#.#.#.#" string for static IP addresses or
// set to NULL for DHCP to be used instead.
#define HTTP_STATIC_IP_ADDRESS      NULL //"192.168.0.128"
#define HTTP_STATIC_SUBNET_MASK     NULL //"255.255.255.0"
#define HTTP_STATIC_GATEWAY_ADDRESS NULL //"192.168.0.1"

// Name of this device to be use with DHCP server.
#define HTTP_HOST_NAME   "MBED"

// Port to which the HTTP server is to be bound.  The well known port number
// for HTTP is 80.
#define HTTP_PORT       80

// String used to identify this sample HTTP server to the client.
#define HTTP_SERVER_NAME "lwIPHTTPServer/1.0"

// Root name of filesystem to be used for HTTP server sample.
#define ROOT_FILESYSTEM_NAME "SD/webfs"

// Where the SHttpServer structure containing the HTTP_BUFFER_POOL should be
// placed in the device's memory.
#define HTTP_MEM_POSITION __attribute((section("AHBSRAM0"),aligned))

// HTTP_PROFILE to 1/0 to enable/disable memory and CPU performance profiling.
#define HTTP_PROFILE    0

// Fakes data for GET responses of specified size.  Set to 0 to use real file system instead.
#define HTTP_DUMMY_DATA_SIZE 0 //(10 * 1024 * 1024)

// Utility macro to determine the element count of an array.
#define ARRAYSIZE(X) (sizeof(X)/sizeof(X[0]))



// GET request handler context.
class CGetRequestHandlerContext : public IHTTPRequestHandlerContext
{
public:
    // Constructors / Destructors
    CGetRequestHandlerContext(FILE*       pFile, 
                              const char* pStatusLine,
                              size_t      StatusLineSize,
                              const char* pHeaders, 
                              size_t      HeaderSize)
    {
        m_pFile = pFile;
        m_pStatusLine = pStatusLine;
        m_StatusLineSize = StatusLineSize;
        m_pHeaders = pHeaders;
        m_HeaderSize = HeaderSize;
    }
    ~CGetRequestHandlerContext()
    {
        // Make sure that file handle isn't being leaked.
        assert ( !m_pFile );
        m_pHeaders = NULL;
        m_pStatusLine = NULL;
        m_HeaderSize = 0;
        m_StatusLineSize = 0;
    }
    
    // IHTTPRequestHandlerContext interface methods.
    virtual int  BeginRequestHeaders()
    {
        return 1;
    }
    virtual void WriteRequestHeader(const char* pHeaderName, 
                                    const char* pHeaderValue)
    {
        printf("%s: %s\n", pHeaderName, pHeaderValue);
    }
    
    virtual void EndRequestHeaders()
    {
    }
    
    virtual int BeginRequestContent(size_t ContentSize)
    {
        // Ignore request content body.
        (void)ContentSize;
        return 0;
    }
    
    virtual void WriteRequestContentBlock(const void* pBlockBuffer, 
                                          size_t BlockBufferSize)
    {
        (void)pBlockBuffer;
        (void)BlockBufferSize;
        assert ( FALSE );
    }
    
    virtual void EndRequestContent()
    {
        assert ( FALSE );
    }
    
    virtual const char* GetStatusLine(size_t* pStatusLineLength)
    {
        *pStatusLineLength = m_StatusLineSize;
        return m_pStatusLine;
    }
    
    virtual const char* GetResponseHeaders(size_t* pResponseHeaderLength)
    {
        *pResponseHeaderLength = m_HeaderSize;
        return m_pHeaders;
    }
    
    virtual int BeginResponseContent()
    {
        assert (m_pFile);
        
        // We have data to be sent back to client.
        return 1;
    }
    
    virtual size_t ReadResponseContentBlock(char*  pBuffer,
                                            size_t BytesToRead)
    {
        return fread(pBuffer, 1, BytesToRead, m_pFile);
    }
    
    virtual void EndResponseContent()
    {
        if (m_pFile)
        {
            fclose(m_pFile);
            m_pFile = NULL;
        }
    }
    
    virtual void Release()
    {
        EndResponseContent();
        delete this;
    }

protected:
    FILE*       m_pFile;
    const char* m_pStatusLine;
    const char* m_pHeaders;
    size_t      m_StatusLineSize;
    size_t      m_HeaderSize;
};


// Class to test application request handler overrides.
class CTestRequestHandler : public IHTTPRequestHandler
{
public:
    // Constructors/Destructors
    CTestRequestHandler(const char* pRootPathname);
    
    // IHTTPRequestHandler interface methods.
    virtual IHTTPRequestHandlerContext* HandleGetRequest(const char* pURI);
    virtual IHTTPRequestHandlerContext* HandleHeadRequest(const char* pURI);
    virtual IHTTPRequestHandlerContext* HandlePostRequest(const char* pURI);
    virtual IHTTPRequestHandlerContext* HandleBadRequest(const char* pRequest);
    
protected:
    const char* m_pRootPathname;
};


CTestRequestHandler::CTestRequestHandler(const char* pRootPathname)
{
    m_pRootPathname = pRootPathname;
}


IHTTPRequestHandlerContext* CTestRequestHandler::HandleGetRequest(const char* pURI)
{
    // Just handle root HTML file and let the default handler handle the rest.
    if (0 == strcmp(pURI, "/"))
    {
        char FilePath[64];
        FILE* pFile;
        
        snprintf(FilePath, ARRAYSIZE(FilePath), "/%s/index.html", m_pRootPathname);
        pFile = fopen(FilePath, "r");
        if (pFile)
        {
            static const char StatusLine[] = "HTTP/1.0 200 OK\r\n";
            static const char Headers[] = "Content-type: text/html\r\n";
                                            
            CGetRequestHandlerContext* pRequestHandlerContext = new CGetRequestHandlerContext(pFile,
                                                                                              StatusLine,
                                                                                              sizeof(StatusLine)-1,
                                                                                              Headers,
                                                                                              sizeof(Headers)-1);
            if (!pRequestHandlerContext)
            {
                fclose(pFile);
            }
            
            return pRequestHandlerContext;
        }
    }

    return NULL;
}

IHTTPRequestHandlerContext* CTestRequestHandler::HandleHeadRequest(const char* pURI)
{
    // Unused parameters.
    (void)pURI;
    
    return NULL;
}

IHTTPRequestHandlerContext* CTestRequestHandler::HandlePostRequest(const char* pURI)
{
    // Unused parameters.
    (void)pURI;
    
    return NULL;
}

IHTTPRequestHandlerContext* CTestRequestHandler::HandleBadRequest(const char* pRequest)
{
    // Unused parameters.
    (void)pRequest;
    
    return NULL;
}


// Function Prototypes
static void        DisplayRAMBanks(void);
static void        DisplayLWIPConfig(void);



int main() 
{
    int                                     Result = 1;
    unsigned int                            BlinkCount = 0;
    int                                     NetworkCPUTime = 0;
    DigitalOut                              ProgressLED(LED1);
    Timer                                   BlinkTimer;
    Timer                                   TotalCPUTimer;
    Timer                                   NetworkCPUTimer;
    static SNetwork                         Network;
    static HTTP_MEM_POSITION CHTTPServer    HTTPServer;
    static CTestRequestHandler              TestRequestHandler(ROOT_FILESYSTEM_NAME);
    
    printf("\r\n" APPLICATION_NAME "\r\n");
    
    // Dump lwIP configuration parameters and RAM banks to stdout.
    DisplayLWIPConfig();
    DisplayRAMBanks();
    
#if HTTP_DUMMY_DATA_SIZE
    // Create a fake file system for performance testing only.
    static FakeFileSystem Fake("SD", HTTP_DUMMY_DATA_SIZE);
#else // 0 == HTTP_DUMMY_DATA_SIZE
    // Create the SD based file system.
    static SDFileSystem sd(p5, p6, p7, p8, "SD");
#endif // HTTP_DUMMY_DATA_SIZE

    // Initialize the ethernet driver and lwIP network stack.
    Result = SNetwork_Init(&Network, 
                           HTTP_STATIC_IP_ADDRESS, 
                           HTTP_STATIC_SUBNET_MASK,
                           HTTP_STATIC_GATEWAY_ADDRESS,
                           HTTP_HOST_NAME);
    if (Result)
    {
        error("HTTP: Application failed to initialize network.\r\n");
    }
    
    // Register test request handler for handling HTTP requests.
    HTTPServer.AttachRequestHandler(&TestRequestHandler);

    // Setup a HTTP server to listen on port 80.
    Result = HTTPServer.Bind(ROOT_FILESYSTEM_NAME, HTTP_SERVER_NAME, HTTP_PORT);
    if (Result)
    {
        error("HTTP: Application failed to initialize for receiving HTTP "
              "requests.\r\n");
    }
                
    // Enter the main application loop.
    BlinkTimer.start();
    TotalCPUTimer.start();
    NetworkCPUTimer.start();
    while(1) 
    {
        int NetworkTasksRun;
        
        // Allow the network stack to do its thing.  It will callback into the
        // application callbacks as necessary to complete the actual work
        // required for a simple HTTP server implementation.
        NetworkCPUTimer.reset();
        NetworkTasksRun = SNetwork_Poll(&Network);
        
        // Count time spent in network if at least one networking related action
        // was performed during the Poll.  If not, it is time that the main
        // application could have been safely doing something else.
        if (NetworkTasksRun)
        {
            NetworkCPUTime += NetworkCPUTimer.read_us();
        }
        
        // Blink the progress LED once each half second to let user know that
        // we haven't hung.
        if (BlinkTimer.read_ms() > 500)
        {
            BlinkTimer.reset();
            ProgressLED = !ProgressLED;
            BlinkCount++;
            
            // Dump lwIP stats on a regular basis.
            if (HTTP_PROFILE && 0 == (BlinkCount & 0x7F))
            {
                printf("\r\nDumping lwIP stats...\r\n");
                stats_display();
            }
        }
        
        // Dump CPU times every 5 seconds.
        if (HTTP_PROFILE && TotalCPUTimer.read_ms() > 5000)
        {
            int TotalCPUTime = TotalCPUTimer.read_us();
            
            printf("\r\nNetwork used %d usecs of %d usecs (%.2f%%)\r\n",
                   NetworkCPUTime,
                   TotalCPUTime,
                   ((float)NetworkCPUTime / (float)TotalCPUTime) * 100.0f);
            TotalCPUTimer.reset();
            NetworkCPUTime = 0;
        }
    }
}


// These external symbols are maintained by the linker to indicate the
// location of various regions in the device's memory.  They will be used by
// DisplayRAMBanks() to dump the size of each RAM bank to stdout.
extern unsigned int Image$$RW_IRAM1$$Base;
extern unsigned int Image$$RW_IRAM1$$ZI$$Limit;
extern unsigned int Image$$RW_IRAM2$$Base;
extern unsigned int Image$$RW_IRAM2$$ZI$$Limit;
extern unsigned int Image$$RW_IRAM3$$Base;
extern unsigned int Image$$RW_IRAM3$$ZI$$Limit;


// Displays the size of static allocations for each RAM bank as indicated by
// ARM linker to stdout.
static void DisplayRAMBanks(void)
{
    printf("Static RAM bank allocations\r\n");
    printf("  Main RAM = %u\r\n", (unsigned int)&Image$$RW_IRAM1$$ZI$$Limit - 
                                  (unsigned int)&Image$$RW_IRAM1$$Base);
    printf("  RAM0     = %u\r\n", (unsigned int)&Image$$RW_IRAM2$$ZI$$Limit -
                                  (unsigned int)&Image$$RW_IRAM2$$Base);
    printf("  RAM1     = %u\r\n", (unsigned int)&Image$$RW_IRAM3$$ZI$$Limit -
                                  (unsigned int)&Image$$RW_IRAM3$$Base);
}


// Displays lwIP configuration parameters to stdout.
static void DisplayLWIPConfig(void)
{
    // lwIP Configuration parameters to be displayed to stdout.
    #define LWIP_SETTING(X) { #X, (unsigned int)X }
    static const struct
    {
        const char*     pName;
        unsigned int    Value;
    } lwIPSettings[] =
    {
        LWIP_SETTING(MEM_SIZE),
        LWIP_SETTING(TCP_MSS),
        LWIP_SETTING(TCP_WND),
        LWIP_SETTING(TCP_QUEUE_OOSEQ),
        LWIP_SETTING(TCP_SND_BUF),
        LWIP_SETTING(TCP_SND_QUEUELEN),
        LWIP_SETTING(TCP_OVERSIZE),
        LWIP_SETTING(TCP_TTL),
        LWIP_SETTING(PBUF_POOL_SIZE),
        LWIP_SETTING(PBUF_POOL_BUFSIZE),
        LWIP_SETTING(PBUF_LINK_HLEN),
        LWIP_SETTING(ARP_TABLE_SIZE),
    };
    size_t  i = 0;
    
    printf("lwIP Configuration\r\n");
    for (i = 0 ; i < ARRAYSIZE(lwIPSettings) ; i++)
    {
        printf("  %s : %u\r\n", lwIPSettings[i].pName, lwIPSettings[i].Value);
    }
}
