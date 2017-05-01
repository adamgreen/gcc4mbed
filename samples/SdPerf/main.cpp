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
/* Test which brings default HelloWorld project from mbed online compiler
   to be built under GCC.
*/
#include <mbed.h>
#include <FATFileSystem.h>
#include "SDBlockDevice.h"


// Chose pins to use for SD card based on target device type.
#if defined(TARGET_LPC1768)
    #define SD_MOSI p5
    #define SD_MISO p6
    #define SD_SCLK p7
    #define SD_CS   p8
#elif defined(TARGET_K64F)
    #define SD_MOSI PTE3
    #define SD_MISO PTE1
    #define SD_SCLK PTE2
    #define SD_CS   PTE4
#else
    #error Target device not currently supported for this sample.
#endif

static void _RecursiveDir(const char* pDirectoryName, DIR* pDirectory = NULL);


int main() 
{

    static SDBlockDevice      sd(SD_MOSI, SD_MISO, SD_SCLK, SD_CS);
    static FATFileSystem      fs("sd", &sd);
    static Timer              timer;
    FILE*                     pFile = NULL;
    size_t                    bytesTransferred = 0;
    size_t                    i = 0;
    int                       seekResult = -1;
    char                      filenameBuffer[256];
    static __attribute((section("AHBSRAM0"),aligned)) unsigned char buffer[16 * 1024];
    static __attribute((section("AHBSRAM1"),aligned)) char          cache[16 * 1024];
    static const unsigned int testFileIterations = 128;
    static const unsigned int testFileSize = testFileIterations * sizeof(buffer);
    

    // Search for a unique filename to test with.
    for (i = 0 ; ; i++)
    {
        snprintf(filenameBuffer, sizeof(filenameBuffer), "/sd/tst%u.bin", i);
        printf("Trying %s...", filenameBuffer);
        pFile = fopen(filenameBuffer, "r");
        if (!pFile)
        {
            printf("free!\n");
            break;
        }
        printf("exists!\n");
        fclose(pFile);
    }
    
    printf("Performing write test...\n");
    memset(buffer, 0x55, sizeof(buffer));
    
    // Write out large file to SD card and time the write.
    pFile = fopen(filenameBuffer, "w");
    if (!pFile)
    {
        printf("error: Failed to create %s", filenameBuffer);
        perror(NULL);
        exit(-1);
    }
    setvbuf(pFile, cache, _IOFBF, sizeof(cache));

    timer.start();
    for (i = 0 ; i < testFileIterations ; i++)
    {
        bytesTransferred = fwrite(buffer, 1, sizeof(buffer), pFile);
        if (bytesTransferred != sizeof(buffer))
        {
            printf("error: Failed to write to %s", filenameBuffer);
            perror(NULL);
            exit(-1);
        }
    }
    unsigned int totalTicks = (unsigned int)timer.read_ms();
    unsigned int totalBytes = ftell(pFile);
    fclose(pFile);

    printf("Wrote %u bytes in %u milliseconds.\n", totalBytes, totalTicks);
    printf("%f bytes/second.\n", totalBytes / (totalTicks / 1000.0f));
    printf("%f MB/second.\n", (totalBytes / (totalTicks / 1000.0f)) / (1024.0f * 1024.0f));


    printf("Performing read test...\n");
    pFile = fopen(filenameBuffer, "r");
    if (!pFile)
    {
        printf("error: Failed to open %s", filenameBuffer);
        perror(NULL);
        exit(-1);
    }
    setvbuf(pFile, cache, _IOFBF, sizeof(cache));
    
    timer.reset();
    for (;;)
    {
        bytesTransferred = fread(buffer, 1, sizeof(buffer), pFile);
        if (bytesTransferred != sizeof(buffer))
        {
            if (ferror(pFile))
            {
                printf("error: Failed to read from %s", filenameBuffer);
                perror(NULL);
                exit(-1);
            }
            else
            {
                break;
            }
        }
    }
    totalTicks = (unsigned int)timer.read_ms();
    totalBytes = ftell(pFile);

    printf("Read %u bytes in %u milliseconds.\n", totalBytes, totalTicks);
    printf("%f bytes/second.\n", totalBytes / (totalTicks / 1000.0f));
    printf("%f MB/second.\n", (totalBytes / (totalTicks / 1000.0f)) / (1024.0f * 1024.0f));
    
    
    printf("Validating data read.  Not for performance measurement.\n");
    seekResult = fseek(pFile, 0, SEEK_SET);
    if (seekResult)
    {
        perror("error: Failed to seek to beginning of file");
        exit(-1);
    }
    
    for (;;)
    {
        unsigned int   j;
        unsigned char* pCurr;
        
        memset(buffer, 0xaa, sizeof(buffer));
        bytesTransferred = fread(buffer, 1, sizeof(buffer), pFile);
        if (bytesTransferred != sizeof(buffer) && ferror(pFile))
        {
            printf("error: Failed to read from %s", filenameBuffer);
            perror(NULL);
            exit(-1);
        }
        
        for (j = 0, pCurr = buffer ; j < bytesTransferred ; j++)
        {
            if (*pCurr++ != 0x55)
            {
                printf("error: Unexpected read byte encountered.");
                exit(-1);
            }
        }
        
        if (bytesTransferred != sizeof(buffer))
            break;
    }
    totalBytes = ftell(pFile);
    printf("Validated %u bytes.\n", totalBytes);
    fclose(pFile);

    printf("Determine size of file through fseek and ftell calls.\n");
    pFile = fopen(filenameBuffer, "r");
    seekResult = fseek(pFile, 0, SEEK_END);
    long size = ftell(pFile);
    if ((long)testFileSize != size)
    {
        printf("error: ftell returned %ld instead of the expected value of %u.\n", size, testFileSize);
        exit(-1);
    }
    fclose(pFile);
    
    printf("Create directories.\n");
    remove("/sd/testdir1/z");
    remove("/sd/testdir1/a");
    remove("/sd/testdir1");
    int mkdirResult = mkdir("/sd/testdir1", 0);
    if (mkdirResult)
    {
        perror("error: mkdir(/sd/testdir1) failed");
        exit(-1);
    }
    mkdirResult = mkdir("/sd/testdir1/a", 0);
    if (mkdirResult)
    {
        perror("error: mkdir(/sd/testdir1/a) failed");
        exit(-1);
    }
    mkdirResult = mkdir("/sd/testdir1/z", 0);
    if (mkdirResult)
    {
        perror("error: mkdir(/sd/testdir1/z) failed");
        exit(-1);
    }

    // Enumerate all content on mounted file systems.
    printf("\nList all files in /sd...\n");
    _RecursiveDir("/sd");

    printf("Cleanup test directories.\n");
    int removeResult = remove("/sd/testdir1/z");
    if (removeResult)
    {
        perror("error: remove(sd/testdir1/z) failed");
        exit(-1);
    }
    removeResult = remove("/sd/testdir1/a");
    if (removeResult)
    {
        perror("error: remove(sd/testdir1/a) failed");
        exit(-1);
    }
    removeResult = remove("/sd/testdir1");
    if (removeResult)
    {
        perror("error: remove(sd/testdir1) failed");
        exit(-1);
    }
    
    printf("Remove test file.\n");
    removeResult = remove(filenameBuffer);
    if (removeResult)
    {
        perror("error: remove() failed");
        exit(-1);
    }

    return 0;
}


static void _RecursiveDir(const char* pDirectoryName, DIR* pDirectory /*= NULL*/)
{
    DIR* pFreeDirectory = NULL;
    
    size_t DirectoryNameLength = strlen(pDirectoryName);
 
    // Open the specified directory.
    if (!pDirectory)
    {
        pDirectory = opendir(pDirectoryName);
        if (!pDirectory)
        {
            fprintf(stderr, "Failed to open directory '%s' for enumeration.\r\n", pDirectoryName);
            exit(-1);
        }
        
        // Remember to free this directory enumerator.
        pFreeDirectory = pDirectory;
    }
        
    // Iterate though each item contained within this directory and display
    // it to the console.
    struct dirent* DirEntry;
    while((DirEntry = readdir(pDirectory)) != NULL) 
    {
        if (0 != strcmp(DirEntry->d_name, ".") &&
            0 != strcmp(DirEntry->d_name, ".."))
        {
            char RecurseDirectoryName[256];
            DIR* pSubdirectory;

            printf("    %.*s/%s\n", 
                   DirectoryNameLength, 
                   pDirectoryName, 
                   DirEntry->d_name);

            // Try opening this file as a directory to see if it succeeds or not.
            snprintf(RecurseDirectoryName, sizeof(RecurseDirectoryName),
                     "%.*s/%s",
                     DirectoryNameLength,
                     pDirectoryName,
                     DirEntry->d_name);
            pSubdirectory = opendir(RecurseDirectoryName);
            if (pSubdirectory)
            {
                _RecursiveDir(RecurseDirectoryName, pSubdirectory);
                closedir(pSubdirectory);
                continue;
            }
        }
    }
    
    // Close the directory enumerator if it was opened by this call.
    if (pFreeDirectory)
    {
        closedir(pFreeDirectory);
    }
}
