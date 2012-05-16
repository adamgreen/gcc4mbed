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
/* Specifies the classes used to implement the FakeFileSystem which is a
   read-only file system that exists in the internal Fake of the mbed
   device.
*/
#include <mbed.h>
#include <assert.h>
#include "FakeFileSystem.h"


FakeFileSystemFileHandle::FakeFileSystemFileHandle(size_t FileSize)
{
    SetEntry(FileSize);
}

FakeFileSystemFileHandle::FakeFileSystemFileHandle()
{
    SetEntry(0);
}
    
ssize_t FakeFileSystemFileHandle::write(const void* pBuffer, size_t Length)
{
    // This file system doesn't support writing.
    return -1;
}


int FakeFileSystemFileHandle::close()
{
    SetEntry(0);
    
    return 0;
}

ssize_t FakeFileSystemFileHandle::read(void* pBuffer, size_t Length)
{
    unsigned int    BytesLeft;

    // Don't read more bytes than what are left in the file.
    BytesLeft = m_Size - m_Offset;
    if (Length > BytesLeft)
    {
        Length = BytesLeft;
    }
    
    // Just leave buffer as is since we are just faking file reads.
    
    // Update the file pointer.
    m_Offset += Length;
    
    return Length;
}

int FakeFileSystemFileHandle::isatty()
{
    return 0;
}

off_t FakeFileSystemFileHandle::lseek(off_t offset, int whence)
{
    switch(whence)
    {
    case SEEK_SET:
        m_Offset = offset;
        break;
    case SEEK_CUR:
        m_Offset += offset;
        break;
    case SEEK_END:
        m_Offset = (m_Size - 1) + offset;
        break;
    default:
        return -1;
    }
    
    return m_Offset;
}

int FakeFileSystemFileHandle::fsync()
{
    return 0;
}

off_t FakeFileSystemFileHandle::flen()
{
    return m_Size;
}




FakeFileSystem::FakeFileSystem(const char* pName, size_t FakeFileSize) : FileSystemLike(pName)
{
    m_FakeFileSize = FakeFileSize;
}

FileHandle* FakeFileSystem::open(const char* pFilename, int Flags)
{
    // Can't find the file if file system hasn't been mounted.
    if (!IsMounted())
    {
        return NULL;
    }

    // Can only open fake files for read.
    if (O_RDONLY != Flags)
    {
        return NULL;
    }
    
    // Attempt to find a free file handle.
    FakeFileSystemFileHandle* pFileHandle = FindFreeFileHandle();
    if (!pFileHandle)
    {
        return NULL;
    }
    
    // Initialize the file handle and return it to caller.
    pFileHandle->SetEntry(m_FakeFileSize);
    return pFileHandle;
}

DirHandle*  FakeFileSystem::opendir(const char *pDirectoryName)
{
    return NULL;
}

FakeFileSystemFileHandle* FakeFileSystem::FindFreeFileHandle()
{
    size_t  i;
    
    // Iterate through the file handle array, looking for a close file handle.
    for (i = 0 ; i < sizeof(m_FileHandles)/sizeof(m_FileHandles[0]) ; i++)
    {
        if (m_FileHandles[i].IsClosed())
        {
            return &(m_FileHandles[i]);
        }
    }
    
    // If we get here, then no free entries were found.
    return NULL;
}
