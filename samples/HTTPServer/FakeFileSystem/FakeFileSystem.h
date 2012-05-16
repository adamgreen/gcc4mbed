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
/* Specifies the classes used to implement the FakeFileSystem which is used by
   the HTTPServer during performance testing to just simulate enough of a 
   filesystem to just send a fixed amount of uninitialized data to the client.
*/
#ifndef _FAKEFILESYSTEM_H_
#define _FAKEFILESYSTEM_H_

#include "FileSystemLike.h"


// Forward declare file system entry structure used internally in 
// FakeFileSystem.
struct _SFileSystemEntry;



// Represents an opened file object in the FakeFileSystem.
class FakeFileSystemFileHandle : public FileHandle 
{
public:
    FakeFileSystemFileHandle();
    FakeFileSystemFileHandle(size_t FileSize);
    
    // FileHandle interface methods.
    virtual ssize_t write(const void* buffer, size_t length);
    virtual int close();
    virtual ssize_t read(void* buffer, size_t length);
    virtual int isatty();
    virtual off_t lseek(off_t offset, int whence);
    virtual int fsync();
    virtual off_t flen();

    // Used by FakeFileSystem to maintain entries in its handle table.
    void SetEntry(size_t FileSize)
    {
        m_Offset = 0;
        m_Size = FileSize;
    }
    int IsClosed()
    {
        return (0 == m_Size);
    }
    
protected:
    // Current position in file to be updated by read and seek operations.
    size_t              m_Offset;
    // The simulated size of this fake file.
    size_t              m_Size;
};


class FakeFileSystem : public FileSystemLike 
{
public:
    FakeFileSystem(const char* pName, size_t FakeFileSize);
    
    virtual FileHandle* open(const char* pFilename, int Flags);
    virtual DirHandle*  opendir(const char *pDirectoryName);

    virtual int         IsMounted() { return 1; }

protected:
    FakeFileSystemFileHandle*  FindFreeFileHandle();
    
    // File handle table used by this file system so that it doesn't need
    // to dynamically allocate file handles at runtime.
    FakeFileSystemFileHandle   m_FileHandles[16];
    
    size_t                     m_FakeFileSize;
};

#endif // _FAKEFILESYSTEM_H_
