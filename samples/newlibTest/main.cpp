/* Copyright 2017 Adam Green (http://mbed.org/users/AdamGreen/)

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
// Basic unit test for C Standard I/O routines.
#include <errno.h>
#include <reent.h>
#include <mbed.h>
#include <mri.h>
#include <FATFileSystem.h>
#include "SDBlockDevice.h"



// Chose pins to use for SD card and secondary UART based on target device type.
#if defined(TARGET_LPC1768)
    #define SD_MOSI     p5
    #define SD_MISO     p6
    #define SD_SCLK     p7
    #define SD_CS       p8
    #define SERIAL_TX   p28
    #define SERIAL_RX   p27
#elif defined(TARGET_K64F)
    #define SD_MOSI     PTE3
    #define SD_MISO     PTE1
    #define SD_SCLK     PTE2
    #define SD_CS       PTE4
    #define SERIAL_TX   PTC17
    #define SERIAL_RX   PTC16
#elif defined(TARGET_KL25Z)
    #define SERIAL_TX   PTE0
    #define SERIAL_RX   PTE1
#elif defined(TARGET_NUMAKER_PFM_M453)
    #define SERIAL_TX   PD_1
    #define SERIAL_RX   PD_6
#elif defined(TARGET_NUMAKER_PFM_NUC472)
    #define SERIAL_TX   PG_2
    #define SERIAL_RX   PG_1
#endif

// Tests will drop into the debugger immediately if they encounter an issue so that I can debug it.
#undef assert
#define assert(X)          if (!(X)) __debugbreak()

// Utility macro to calculate element count of an array at compile time.
#undef ARRAY_SIZE
#define ARRAY_SIZE(X) (sizeof(X)/sizeof(X[0]))

// The multithreaded malloc() test will allocate MALLOC_ALLOC_COUNT allocations, each MALLOC_ALLOC_SIZE bytes
// in length before it frees them and does it all over again.
#if defined(TARGET_KL25Z)
    #define MALLOC_ALLOC_COUNT 64
#else
    #define MALLOC_ALLOC_COUNT 246
#endif
#define MALLOC_ALLOC_SIZE  8

// The size of the thread stacks are proportional to MALLOC_ALLOC_COUNT, the number of pointers to be stored on the
// stack from malloc() calls.
#define THREAD_STACK_SIZE  (MALLOC_ALLOC_COUNT * sizeof(void*) + 512)
#define THREAD_AUTO_ALLOC  NULL



static Thread         g_thread1(osPriorityNormal, THREAD_STACK_SIZE, THREAD_AUTO_ALLOC, "Thread1");
static Thread         g_thread2(osPriorityNormal, THREAD_STACK_SIZE, THREAD_AUTO_ALLOC, "Thread2");
static Semaphore      g_allocationsDoneSemaphore(0, 2);
static Semaphore      g_allocationsStartSemaphore(0, 2);
static Timer          g_timer;

// MRI uses semihosting to redirect stdout/stderr/stdin to GDB and setting up this serial port here would conflict
// with MRI's use of the UART.
#if !MRI_ENABLE
static RawSerial      g_pc(USBTX, USBRX);
#endif // !MRI_ENABLE



// Forward Declarations
static void runPrintfTest();
static void printfThread1();
static void printfThread2();
static void runMallocTest();
static void mallocTestThread();
static void validateHeap();
static void runErrNoTest();
static void mallocFailThread();
static void runSysCallTests();
static void runSvcUserTests();
static uint32_t callSvc1(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
static uint32_t SVC1_Handler(uint32_t a, uint32_t b, uint32_t c, uint32_t d);



int main()
{
#if !MRI_ENABLE
    g_pc.baud(230400);
#endif // MRI_ENABLE

    printf("\n\n");
    while (1)
    {
        const int maxTestId = 5;
        char      lineInput[16];
        
        printf("1) printf Test\n");
        printf("2) malloc Test\n");
        printf("3) errno Test\n");
        printf("4) syscall Tests\n");
        printf("5) SVC_User Tests\n");
        printf("Select test to run (1 - %d): ", maxTestId);

        fgets(lineInput, sizeof(lineInput), stdin);
        int selection = atoi(lineInput);

        switch (selection)
        {
        case 1:
            printf("\n\nStarting printf Test\n");
            runPrintfTest();
            break;
        case 2:
            printf("\n\nStarting malloc Test\n");
            runMallocTest();
            break;
        case 3:
            printf("\n\nStarting errno Test\n");
            runErrNoTest();
            break;
        case 4:
            printf("\n\nStarting syscall Tests\n");
            runSysCallTests();
            break;
        case 5:
            printf("\n\nStarting SVC_User Tests\n");
            runSvcUserTests();
            break;
        default:
            printf("\nInvalid selection\n\n");
            break;
        }
    }
}



// Use printf() from 3 threads (including main) at the same time.
static void runPrintfTest()
{
    g_thread1.start(printfThread1);
    g_thread2.start(printfThread2);
    
    for(uint32_t i = 0 ; ; i++)
        printf("0-%lu\n", i);
}

static void printfThread1()
{
    for(uint32_t i = 0 ; ; i++)
        printf("1-%lu\n", i);
}

static void printfThread2()
{
    for(uint32_t i = 0 ; ; i++)
        printf("2-%lu\n", i);
}



// Use malloc() and free() from 2 threads at the same time.
// The threads pause after each second to allow the main thread to walk the heap and make sure that nothing has
// obviously been corrupted and then resumes execution for another second.
static void runMallocTest()
{
    g_timer.reset();
    g_timer.start();

    g_thread1.start(mallocTestThread);
    g_thread2.start(mallocTestThread);
    for (;;)
    {
        // Wait for both threads to finish.
        g_allocationsDoneSemaphore.wait();
        g_allocationsDoneSemaphore.wait();

        printf("Running heap validation pass...\n");
        validateHeap();

        // Let both threads start up again.
        g_timer.reset();
        g_allocationsStartSemaphore.release();
        g_allocationsStartSemaphore.release();
    }
}

static void mallocTestThread()
{
    void* allocations[MALLOC_ALLOC_COUNT];

    // These volatile float counters are here to have the CPU auto-stack FPU registers on a context switch to
    // verify that my context switching updates didn't have an impact on the FPU part of the stack either.
    volatile float value=1.0f;
    for(;;)
    {
        for(size_t i = 0 ; i < sizeof(allocations)/sizeof(allocations[0]) ; i++)
        {
            allocations[i] = malloc(MALLOC_ALLOC_SIZE);
            assert ( allocations[i] != NULL );
        }

        for(size_t i = 0 ; i < sizeof(allocations)/sizeof(allocations[0]) ; i+=2)
        {
            free(allocations[i]);
            allocations[i] = NULL;
        }

        if (g_timer.read_ms() > 1000)
        {
            g_allocationsDoneSemaphore.release();
            g_allocationsStartSemaphore.wait();
        }

        for(size_t i = 1 ; i < sizeof(allocations)/sizeof(allocations[0]) ; i+=2)
        {
            free(allocations[i]);
            allocations[i] = NULL;
        }

        value += 1.0f;
    }
}

#ifdef _REENT_SMALL

extern "C" uint32_t  __end__[];
extern "C" uint32_t  __malloc_free_list;

// Newlib-Nano Version
static void validateHeap()
{
    // The __end__ linker symbol points to the beginning of the heap.
    uint32_t heapStart = (uint32_t)__end__;
    uint32_t chunkCurr = heapStart;
    // Calling _sbrk() with 0 reserves no more memory but it returns the current top of heap.
    uint32_t heapEnd = (uint32_t)_sbrk_r(_REENT, 0);
    
    // Walk through the chunks until we hit the end of the heap.
    while (chunkCurr < heapEnd)
    {
        // Verify that free node falls within heap.
        assert ( chunkCurr >= heapStart );

        // The first 32-bit word in a chunk is the size of the allocation.  newlib-nano over allocates by 8 bytes.
        // 4 bytes for this 32-bit chunk size and another 4 bytes to allow for 8 byte-alignment of returned pointer.
        uint32_t chunkSize = *(uint32_t*)chunkCurr;

        chunkCurr = chunkCurr + chunkSize;
    }
    // Last chunk should fill right up to end of heap.
    assert ( chunkCurr == heapEnd );
    
    // Verify the free list.
    // __malloc_free_list is the head pointer to newlib-nano's link list of free chunks.
    uint32_t freeCurr = __malloc_free_list;
    while (freeCurr)
    {
        // The first 32-bit word in a chunk is the size of the allocation.  newlib-nano over allocates by 8 bytes.
        // 4 bytes for this 32-bit chunk size and another 4 bytes to allow for 8 byte-alignment of returned pointer.
        uint32_t chunkSize = *(uint32_t*)freeCurr;
        // The start of the next chunk is right after the end of this one.
        uint32_t freeEnd = freeCurr + chunkSize;

        // Verify that free node falls within heap.
        assert ( freeCurr < heapEnd && freeCurr >= heapStart );
        assert ( freeEnd <= heapEnd && freeEnd >= heapStart );
        
        // The second 32-bit word in a free chunk is a pointer to the next free chunk (again sorted by address).
        freeCurr = *(uint32_t*)(freeCurr + 4);
    }
}

#else

extern "C" uint32_t  __end__[];
extern "C" uint32_t  __data_start__[];

// Full Newlib Version
static void validateHeap()
{
    const uint32_t inUseBit = 1;
    
    // The __end__ linker symbol points to the beginning of the heap.
    uint32_t* pHeapStart = __end__;
    uint32_t* pRamStart = __data_start__;
    uint32_t* pChunkCurr = pHeapStart;
    uint32_t  lastChunkSize = pChunkCurr[0];
    
    // Calling _sbrk() with 0 reserves no more memory but it returns the current top of heap.
    uint32_t* pHeapEnd = (uint32_t*)_sbrk_r(_REENT, 0);
    
    // Walk through the chunks until we hit the end of the heap.
    while (pChunkCurr < pHeapEnd)
    {
        // The second 32-bit word in a chunk is the size of the current allocation. Its least significant bit indicates
        // whether the previous chunk is InUse (1) or Free (0).
        uint32_t chunkSize = pChunkCurr[1];
        uint32_t prevChunkInUse = chunkSize & inUseBit;
        chunkSize &= ~inUseBit;

        // The allocations are over allocated by 8 bytes to hold a header and the minimum data size is 8-bytes so that 
        // it can hold forward and backward links when on the free list. The size should always be 8-byte aligned.
        assert ( chunkSize >= 16 );
        assert ( (chunkSize & 7) == 0 );

        // Validate the forward and back links on the previous node if it was marked as free.
        if (!prevChunkInUse)
        {
            // The first 32-bit word in a chunk is the size of the previous allocation.
            uint32_t prevChunkSize = pChunkCurr[0];
            assert ( prevChunkSize == lastChunkSize );

            uint32_t* pPrevChunk = (uint32_t*)((uint8_t*)pChunkCurr - prevChunkSize);
            assert ( pPrevChunk >= pHeapStart && pPrevChunk < pHeapEnd );

            // The forward/back links should point to another entry in heap or to the sized bins lower down in RAM.
            uint32_t* pForwardLink = (uint32_t*)pPrevChunk[2];
            uint32_t* pBackLink = (uint32_t*)pPrevChunk[3];
            assert ( pForwardLink >= pRamStart && pForwardLink < pHeapEnd );
            assert ( pBackLink >= pRamStart && pBackLink < pHeapEnd );
        }

        // Advance to next chunk on the heap.
        lastChunkSize = chunkSize;
        pChunkCurr = (uint32_t*)((uint8_t*)pChunkCurr + chunkSize);
    }
    // Last chunk should fill right up to end of heap.
    assert ( pChunkCurr == pHeapEnd );
}
#endif



// Have 2 threads generating different error codes in errno and make sure that they don't have contention like they
// did before.
static void runErrNoTest()
{
    g_thread1.start(mallocFailThread);
    
    while (true) 
    {
        FILE* pFile = fopen("/foo/bar.txt", "r");
        assert ( !pFile );
        assert ( errno == ENOENT );
    }
}

static void mallocFailThread()
{
    while (true) 
    {
        void* pv = malloc(128 * 1024 * 1024);
        assert ( !pv );
        assert ( errno == ENOMEM );
    }
}



// Forward declare some of the Posix routines in newlib rather than pull in their corresponding headers since some
// conflict with mbed headers.
extern "C" int   isatty(int __fildes );
extern "C" int   execve(const char *__path, char * const __argv[], char * const __envp[]);
extern "C" pid_t fork(void);
extern "C" pid_t getpid(void);
#include <sys/signal.h>
#include <sys/times.h>

// Issue various Standard C Library / Posix calls to generate calls to the updated syscalls in mbed_retarget.cpp.
// It attempts to generate as many of the successful and error code paths through these syscalls as possible so that
// I could step through most of mbed_retarget.cpp.
static void runSysCallTests()
{
    FILE*           pFile = NULL;
    DIR*            pDir = NULL;
    int             bytesWritten = -1;
    int             bytesRead = -1;
    int             isattyResult = -1;
    int             seekResult = -1;
    int             closeResult = -1;
    int             removeResult = -1;
    int             statResult = -1;
    int             mkdirResult = -1;
    struct stat     stats;
    unsigned char   readBuffer[256];


#ifdef SD_MOSI
    static const char  filename[] = "/sd/foo.bar";
    SDBlockDevice      sd(SD_MOSI, SD_MISO, SD_SCLK, SD_CS);
    FATFileSystem      fs("sd", &sd);
    unsigned char      testBuffer[256];
    
    // Fill in test buffer with every byte value possible.
    for (size_t i = 0 ; i < sizeof(testBuffer) ; i++)
    {
        testBuffer[i] = i;
    }
    
    // Create a file in the SD based file system with this data.
    pFile = fopen(filename, "w");
    assert ( pFile );
    
    bytesWritten = fwrite(testBuffer, 1, sizeof(testBuffer), pFile);
    assert ( bytesWritten == sizeof(testBuffer) );
    
    closeResult = fclose(pFile);
    assert ( closeResult == 0 );
    pFile = NULL;
    
    // Now reopen the file and read in the data and make sure it matches.
    pFile = fopen(filename, "r");
    assert ( pFile );
    
    memset(readBuffer, 0, sizeof(readBuffer));
    bytesRead = fread(readBuffer, 1, sizeof(readBuffer), pFile);
    assert (bytesRead == sizeof(readBuffer) );
    assert ( 0 == memcmp(testBuffer, readBuffer, sizeof(testBuffer)) );
    
    closeResult = fclose(pFile);
    assert ( closeResult == 0 );
    pFile = NULL;

    // Reopen the file in binary mode and make sure that the test still works since it should get masked off in
    // openmode_to_posix().
    pFile = fopen(filename, "rb");
    assert ( pFile );
    
    memset(readBuffer, 0, sizeof(readBuffer));
    bytesRead = fread(readBuffer, 1, sizeof(readBuffer), pFile);
    assert (bytesRead == sizeof(readBuffer) );
    assert ( 0 == memcmp(testBuffer, readBuffer, sizeof(testBuffer)) );
    
    closeResult = fclose(pFile);
    assert ( closeResult == 0 );
    pFile = NULL;

    // Reopen the file and use fseek()/ftell to determine file size.
    pFile = fopen(filename, "r");
    assert ( pFile );

    seekResult = fseek(pFile, 0, SEEK_END);
    assert ( seekResult == 0 );
    
    long fileSize = ftell(pFile);
    assert ( fileSize = sizeof(testBuffer) );

    // Calling fstat on this file should set errno to EBADF.
    errno = 0;
    int fstatResult = fstat(fileno(pFile), &stats);
    assert ( fstatResult == -1 );
    assert ( errno == EBADF );

    // This file should not be a TTY.
    isattyResult = isatty(fileno(pFile));
    assert ( !isattyResult );

    // stdout should be a TTY though.
    isattyResult = isatty(fileno(stdout));
    assert ( isattyResult );

    // Close up the file now that we are done with it.
    closeResult = fclose(pFile);
    assert ( closeResult == 0 );
    pFile = NULL;

    // Calling stat on this file by name.
    statResult = stat(filename, &stats);
    assert ( statResult == 0 );

    // Open a file for read and then attempt to use the sycall directly to write to it. Should fail.
    pFile = fopen(filename, "r");
    assert ( pFile );

    errno = 0;
    bytesWritten = _write_r(_REENT, fileno(pFile), testBuffer, sizeof(testBuffer));
    assert ( bytesWritten < 0 );
    assert ( errno = EACCES );

    closeResult = fclose(pFile);
    assert ( closeResult == 0 );
    
    // Open a file for write-only and then attempt to use the sycall directly to read from it. Should fail.
    pFile = fopen(filename, "w");
    assert ( pFile );

    errno = 0;
    bytesRead = _read_r(_REENT, fileno(pFile), testBuffer, sizeof(testBuffer));
    assert ( bytesRead < 0 );
    assert ( errno = EACCES );

    closeResult = fclose(pFile);
    assert ( closeResult == 0 );

    // Attempt to rename a file to a non-existing file system. Should fail.
    errno = 0;
    int renameResult = rename(filename, "/foo/bar.txt");
    assert ( renameResult == -1 );
    assert ( errno == EINVAL );

    // Attempt to rename a file from a non-existing file system to this SD one. Should fail.
    errno = 0;
    renameResult = rename("/foo/bar.txt", "/sd/foo.txt");
    assert ( renameResult == -1 );
    assert ( errno == ENOENT );


    // Directory reading tests.
    // Attempt to open a directory on the SD that we know doesn't exist. Should fail.
    errno = 0;
    pDir = opendir("/sd/foo.bar");
    assert ( pDir == NULL );
    assert ( errno == ENOENT );

    // Open the root directory of the SD card.
    pDir = opendir("/sd");
    assert ( pDir != NULL );

    // Read the first entry.
    struct dirent* pDirEntry = readdir(pDir);
    assert ( pDirEntry != NULL );

    // Read the second entry and remember its position.
    pDirEntry = readdir(pDir);
    assert ( pDirEntry != NULL );
    off_t secondPosition = telldir(pDir);
    assert ( secondPosition >= 0 );

    // Seek to second entry and ignore result since I don't think ChaN FAT filesystem supports this anyway.
    seekdir(pDir, secondPosition);
    pDirEntry = readdir(pDir);
    assert ( pDirEntry != NULL );

    // Rewind to beginning and ignore results since I don't think ChaN FAT filesystem supports the way it is being done here.
    rewinddir(pDir);
    pDirEntry = readdir(pDir);
    assert ( pDirEntry != NULL );

    // Continue walking to the end of the directory.
    errno = 0;
    while (pDirEntry)
    {
        pDirEntry = readdir(pDir);
    }
    // Expect to hit end of list with no error.
    assert ( errno == 0 );
    
    // Done walking directory so close it.
    closeResult = closedir(pDir);
    assert ( closeResult == 0 );
    pDir = NULL;

    // Create a test directory.
    remove("/sd/delete"); // Just incase of a previous failure that left it around.
    mkdirResult = mkdir("/sd/delete", 0);
    assert ( mkdirResult == 0 );

    pDir = opendir("/sd/delete");
    assert ( pDir != NULL );

    closeResult = closedir(pDir);
    assert ( closeResult == 0 );
    pDir = NULL;

    // Attempt to create it again and it should fail.
    errno = 0;
    mkdirResult = mkdir("/sd/delete", 0);
    assert ( mkdirResult == -1 );
    assert ( errno == EEXIST );

    removeResult = remove("/sd/delete");
    assert ( removeResult == 0 );

    
    // Can delete the file now that we are done with it.
    removeResult = remove(filename);
    assert ( removeResult == 0 );

    // Attempt to open an existing file in the SD file system which we know doesn't exist.
    errno = 0;
    pFile = fopen("/sd/bar.txt", "r");
    assert ( pFile == NULL );
    assert ( errno = ENOENT );

    // Attempt to remove a file in the SD file system which we know doesn't exist.
    errno = 0;
    removeResult = remove("/sd/bar.txt");
    assert ( removeResult == -1 );
    assert ( errno == ENOENT );

    // Attempt to rename a file in the SD file system which we know doesn't exist.
    errno = 0;
    renameResult = rename("/sd/bar.txt", "/sd/foo.txt");
    assert ( renameResult == -1 );
    assert ( errno == ENOENT );

    // UNDONE: Can't actually run this test since 64 handles ends up using too much heap.
    // It takes over 600 bytes per file handle opened.
#ifdef UNDONE
    // Attempt to open more files than what can fit in the filehandles table.
    char buffer[32];
    FILE* pFiles[OPEN_MAX];
    for (size_t i = 0 ; i < ARRAY_SIZE(pFiles) ; i++)
    {
        snprintf(buffer, sizeof(buffer), "/sd/tstfil%u.txt", i+1);
        pFiles[i] = fopen(buffer, "w");
        assert ( pFiles[i] );
    }

    errno = 0;
    pFile = fopen("/sd/tstfile00.txt", "w");
    assert ( pFile == NULL );
    assert ( errno == EMFILE );
    
    for (size_t i = 0 ; i < ARRAY_SIZE(pFiles) ; i++)
    {
        closeResult = fclose(pFiles[i]);
        assert ( closeResult == 0 );
        pFiles[i] = NULL;
        snprintf(buffer, sizeof(buffer), "/sd/tstfil%u.txt", i+1);
        remove(buffer);
    }
#endif // UNDONE
#endif // SD_MISO

    // Named Serial streams places FileLike objects in the root of the namespace.
    // When stream goes out of scope, it will call mbed::remove_filehandles().
    {
        Serial testSerial(SERIAL_TX, SERIAL_RX, "com2");
        pFile = fopen("/com2", "w+");
        assert ( pFile );
        int setvbufResult = setvbuf(pFile, NULL, _IONBF, 0);
        assert ( setvbufResult == 0 );
    }

    // Write should fail since the underlying stream was closed when testSerial went out of scope.
    errno = 0;
    bytesWritten = fwrite("Hello\n", 1, 6, pFile);
    assert ( bytesWritten == 0 );
    assert ( errno == EBADF );

    // Read should fail since the underlying stream was closed when testSerial went out of scope.
    errno = 0;
    bytesRead = fread(readBuffer, 1, sizeof(readBuffer), pFile);
    assert ( bytesRead == 0 );
    assert ( errno == EBADF );

    // isatty() should fail too.
    errno = 0;
    isattyResult = isatty(fileno(pFile));
    assert ( isattyResult == -1 );
    assert ( errno == EBADF );

    // fseek() should fail too.
    errno = 0;
    seekResult = fseek(pFile, SEEK_CUR, 2);
    assert ( seekResult == -1 );
    assert ( errno == EBADF );

    // fclose() will fail too.
    errno = 0;
    closeResult = fclose(pFile);
    assert ( closeResult == -1 );
    assert ( errno == EBADF );
    pFile = NULL;

    // Attempt to create a file where the root file system name is invalid. Should fail.
    errno = 0;
    pFile = fopen("/foo/bar.txt", "w");
    assert ( pFile == NULL );
    assert ( errno = ENOENT );

    // Attempt to remove a file where the root file system name is invalid. Should fail.
    errno = 0;
    removeResult = remove("/foo/bar.txt");
    assert ( removeResult == -1 );
    assert ( errno = ENOENT );

    // Attempt to stat a file where the root file system name is invalid. Should fail.
    errno = 0;
    statResult = stat("/foo/bar.txt", &stats);
    assert ( statResult == -1 );
    assert ( errno = ENOENT );

    // Attempt to allocate a really large allocation to make sure that it fails. Should fail.
    errno = 0;
    void* pMallocResult = malloc(64 * 1024 * 1024);
    assert ( pMallocResult == NULL );
    assert ( errno == ENOMEM );

    // Attempt to opendir() on an invalid file system. Should fail.
    errno = 0;
    pDir = opendir("/foo");
    assert ( pDir == NULL );
    assert ( errno == ENOENT );

    // Attempt to mkdir() on an invalid file system. Should fail.
    errno = 0;
    mkdirResult = mkdir("/foo/testdir", 0);
    assert ( mkdirResult == -1 );
    assert ( errno == ENOENT );
    

    // There are a few syscalls that are implemented to just return ENOSYS.
    errno = 0;
    assert ( NULL == tmpnam(NULL) );
    assert ( errno == ENOSYS );

    errno = 0;
    assert ( NULL == tmpfile() );
    assert ( errno == ENOSYS );

    errno = 0;
    assert ( -1 == execve("bash", NULL, NULL) );
    assert ( errno == ENOSYS );

    errno = 0;
    assert ( -1 == fork() );
    assert ( errno == ENOSYS );

    errno = 0;
    assert ( -1 == getpid() );
    assert ( errno == ENOSYS );

    errno = 0;
    assert ( -1 == kill(1, 9) );
    assert ( errno == ENOSYS );

    struct tms timesResult;
    errno = 0;
    times(&timesResult);
    assert ( errno == ENOSYS );

    int status = -1;
    errno = 0;
    assert ( -1 == _wait_r(_REENT, &status) );
    assert ( errno == ENOSYS );

    printf("SysCall Tests completed successfully!\n\n");
}



// RTX has a SVC_User feature. This test registers one of these SVC user routines and calls it 1000 times to
// make sure that the changes to the assembly language code down in this area didn't break the functionality.
static void runSvcUserTests()
{
    for (int i = 0 ; i < 1000 ; i++)
    {
        uint32_t a = rand();
        uint32_t b = rand();
        uint32_t c = rand();
        uint32_t d = rand();
        uint32_t sum = callSvc1(a, b, c, d);
        
        assert ( sum == (a + b + c + d) );
    }

    printf("SVC_User Tests completed successfully!\n\n");
}

static uint32_t callSvc1(register uint32_t arg0, register uint32_t arg1, register uint32_t arg2, register uint32_t arg3)
{
    register uint32_t __r0 asm("r0") = arg0;
    register uint32_t __r1 asm("r1") = arg1;
    register uint32_t __r2 asm("r2") = arg2;
    register uint32_t __r3 asm("r3") = arg3;

    asm volatile ("svc 1" : "=r"(__r0) : "r"(__r0), "r"(__r1), "r"(__r2), "r"(__r3) : );

    return __r0;
}

// Tell RTX about our SVC1 handler by defining this osRtxUserSVC table.
extern "C" void* const osRtxUserSVC[2] = { (void *)1, (void*)SVC1_Handler };

static uint32_t SVC1_Handler(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    return a + b + c + d;
}
