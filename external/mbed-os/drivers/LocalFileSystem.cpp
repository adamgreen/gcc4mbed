/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "drivers/LocalFileSystem.h"

#if DEVICE_LOCALFILESYSTEM

#include <sys/errno.h>
#include "platform/mbed_semihost_api.h"

namespace mbed {

/* Extension to FINFO type defined in RTL.h (in Keil RL) - adds 'create time'. */
typedef struct {
    unsigned char  hr;   /* Hours    [0..23]                  */
    unsigned char  min;  /* Minutes  [0..59]                  */
    unsigned char  sec;  /* Seconds  [0..59]                  */
    unsigned char  day;  /* Day      [1..31]                  */
    unsigned char  mon;  /* Month    [1..12]                  */
    unsigned short year; /* Year     [1980..2107]             */
} FTIME;

typedef struct {         /* File Search info record           */
    char  name[32];      /* File name                         */
    long  size;          /* File size in bytes                */
    int   fileID;        /* System File Identification        */
    FTIME create_time;   /* Date & time file was created      */
    FTIME write_time;    /* Date & time of last write         */
} XFINFO;

#define RESERVED_FOR_USER_APPLICATIONS (0x100) /* 0x100 - 0x1ff */
#define USR_XFFIND (RESERVED_FOR_USER_APPLICATIONS + 0)

static int xffind (const char *pattern, XFINFO *info) {
    unsigned param[4];

    param[0] = (unsigned long)pattern;
    param[1] = (unsigned long)strlen(pattern);
    param[2] = (unsigned long)info;
    param[3] = (unsigned long)sizeof(XFINFO);

    return __semihost(USR_XFFIND, param);
}

#define OPEN_R          0
#define OPEN_B          1
#define OPEN_PLUS       2
#define OPEN_W          4
#define OPEN_A          8
#define OPEN_INVALID   -1

int posix_to_semihost_open_flags(int flags) {
    /* POSIX flags -> semihosting open mode */
    int openmode;
    if (flags & O_RDWR) {
        /* a plus mode */
        openmode = OPEN_PLUS;
        if (flags & O_APPEND) {
            openmode |= OPEN_A;
        } else if (flags & O_TRUNC) {
            openmode |= OPEN_W;
        } else {
            openmode |= OPEN_R;
        }
    } else if (flags & O_WRONLY) {
        /* write or append */
        if (flags & O_APPEND) {
            openmode = OPEN_A;
        } else {
            openmode = OPEN_W;
        }
    } else if (flags == O_RDONLY) {
        /* read mode */
        openmode = OPEN_R;
    } else {
        /* invalid flags */
        openmode = OPEN_INVALID;
    }

    return openmode;
}


class LocalFileHandle {

public:
    LocalFileHandle(FILEHANDLE fh);

    int close();

    ssize_t write(const void *buffer, size_t length);

    ssize_t read(void *buffer, size_t length);

    int isatty();

    off_t lseek(off_t position, int whence);

    int fsync();

    off_t flen();

protected:
    void lock();
    void unlock();
    FILEHANDLE _fh;
    int pos;
    PlatformMutex _mutex;
};


LocalFileHandle::LocalFileHandle(FILEHANDLE fh) : _fh(fh), pos(0) {
    // No lock needed in constructor
}

int LocalFileHandle::close() {
    int retval = semihost_close(_fh);
    delete this;
    return retval;
}

ssize_t LocalFileHandle::write(const void *buffer, size_t length) {
    lock();
    ssize_t n = semihost_write(_fh, (const unsigned char*)buffer, length, 0); // number of characters not written
    n = length - n; // number of characters written
    pos += n;
    unlock();
    return n;
}

ssize_t LocalFileHandle::read(void *buffer, size_t length) {
    lock();
    ssize_t n = semihost_read(_fh, (unsigned char*)buffer, length, 0); // number of characters not read
    n = length - n; // number of characters read
    pos += n;
    unlock();
    return n;
}

int LocalFileHandle::isatty() {
    lock();
    int ret = semihost_istty(_fh);
    unlock();
    return ret;
}

off_t LocalFileHandle::lseek(off_t position, int whence) {
    lock();
    if (whence == SEEK_CUR) {
        position += pos;
    } else if (whence == SEEK_END) {
        position += semihost_flen(_fh);
    } /* otherwise SEEK_SET, so position is fine */

    /* Always seems to return -1, so just ignore for now. */
    semihost_seek(_fh, position);
    pos = position;
    unlock();
    return position;
}

int LocalFileHandle::fsync() {
    lock();
    int ret = semihost_ensure(_fh);
    unlock();
    return ret;
}

off_t LocalFileHandle::flen() {
    lock();
    off_t off = semihost_flen(_fh);
    unlock();
    return off;
}

void LocalFileHandle::lock() {
    _mutex.lock();
}

void LocalFileHandle::unlock() {
    _mutex.unlock();
}


class LocalDirHandle {

public:
    struct dirent cur_entry;
    XFINFO info;

    LocalDirHandle() {
        memset(&cur_entry, 0, sizeof(cur_entry));
        memset(&info, 0, sizeof(info));
    }

    int closedir() {
        // No lock can be used in destructor
        delete this;
        return 0;
    }

    struct dirent *readdir() {
        lock();
        if (xffind("*", &info)!=0) {
            unlock();
            return NULL;
        }
        memcpy(cur_entry.d_name, info.name, sizeof(info.name));
        unlock();
        return &cur_entry;
    }

    void rewinddir() {
        lock();
        info.fileID = 0;
        unlock();
    }

    off_t telldir() {
        lock();
        int fileId = info.fileID;
        unlock();
        return fileId;
    }

    void seekdir(off_t offset) {
        lock();
        info.fileID = offset;
        unlock();
    }

protected:
    PlatformMutex _mutex;

    void lock() {
        _mutex.lock();
    }

    void unlock() {
        _mutex.unlock();
    }
};


LocalFileSystem::LocalFileSystem(const char* pName) : FileSystem(pName)
{
}


int LocalFileSystem::mount(BlockDevice *bd)
{
    return ENOTSUP;
}

int LocalFileSystem::mount(BlockDevice *bd, bool force)
{
    return ENOTSUP;
}

int LocalFileSystem::unmount()
{
    return ENOTSUP;
}

int LocalFileSystem::remove(const char *path)
{
    // No global state modified so function is thread safe

    return semihost_remove(path);
}

int LocalFileSystem::rename(const char *path, const char *newpath)
{
    return ENOTSUP;
}

int LocalFileSystem::stat(const char *path, struct stat *st)
{
    return ENOTSUP;
}

int LocalFileSystem::mkdir(const char *path, mode_t mode)
{
    return ENOTSUP;
}



int LocalFileSystem::file_open(fs_file_t* ppFile, const char *pFilename, int flags)
{
    // No global state modified so function is thread safe

    /* reject filenames with / in them */
    for (const char *tmp = pFilename; *tmp; tmp++) {
        if (*tmp == '/') {
            return ENOENT;
        }
    }

    int openmode = posix_to_semihost_open_flags(flags);
    if (openmode == OPEN_INVALID) {
        return EACCES;
    }

    FILEHANDLE fh = semihost_open(pFilename, openmode);
    if (fh == -1) {
        return ENOENT;
    }

    *ppFile = new LocalFileHandle(fh);
    return 0;
}

int LocalFileSystem::file_close(fs_file_t file)
{
    LocalFileHandle* pFile = (LocalFileHandle*)file;
    return pFile->close();
}

ssize_t LocalFileSystem::file_read(fs_file_t file, void *buffer, size_t len)
{
    LocalFileHandle* pFile = (LocalFileHandle*)file;
    return pFile->read(buffer, len);
}

ssize_t LocalFileSystem::file_write(fs_file_t file, const void *buffer, size_t len)
{
    LocalFileHandle* pFile = (LocalFileHandle*)file;
    return pFile->write(buffer, len);
}

int LocalFileSystem::file_sync(fs_file_t file)
{
    LocalFileHandle* pFile = (LocalFileHandle*)file;
    return pFile->fsync();
}

off_t LocalFileSystem::file_seek(fs_file_t file, off_t offset, int whence)
{
    LocalFileHandle* pFile = (LocalFileHandle*)file;
    return pFile->lseek(offset, whence);
}

off_t LocalFileSystem::file_tell(fs_file_t file)
{
    LocalFileHandle* pFile = (LocalFileHandle*)file;
    return pFile->lseek(0, SEEK_CUR);
}

size_t LocalFileSystem::file_size(fs_file_t file)
{
    LocalFileHandle* pFile = (LocalFileHandle*)file;
    return pFile->flen();
}


int LocalFileSystem::dir_open(fs_dir_t* ppDir, const char* pDirectoryName)
{
    // No global state modified so function is thread safe

    *ppDir = new LocalDirHandle();
    return 0;
}

int LocalFileSystem::dir_close(fs_dir_t dir)
{
    LocalDirHandle* pDirHandle = (LocalDirHandle*)dir;
    return pDirHandle->closedir();
}

ssize_t LocalFileSystem::dir_read(fs_dir_t dir, struct dirent *ent)
{
    LocalDirHandle* pDirHandle = (LocalDirHandle*)dir;
    struct dirent* pEntry = pDirHandle->readdir();
    if (pEntry == NULL)
    {
        return 0;
    }
    else
    {
        *ent = *pEntry;
        return 1;
    }
}

void LocalFileSystem::dir_seek(fs_dir_t dir, off_t offset)
{
    LocalDirHandle* pDirHandle = (LocalDirHandle*)dir;
    pDirHandle->seekdir(offset);
}

off_t LocalFileSystem::dir_tell(fs_dir_t dir)
{
    LocalDirHandle* pDirHandle = (LocalDirHandle*)dir;
    return pDirHandle->telldir();
}

void LocalFileSystem::dir_rewind(fs_dir_t dir)
{
    LocalDirHandle* pDirHandle = (LocalDirHandle*)dir;
    pDirHandle->rewinddir();
}

} // namespace mbed

#endif
