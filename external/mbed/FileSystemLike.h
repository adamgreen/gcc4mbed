/* mbed Microcontroller Library
 * Copyright (c) 2006-2012 ARM Limited
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef MBED_FILESYSTEMLIKE_H
#define MBED_FILESYSTEMLIKE_H

#ifdef __ARMCC_VERSION
#    define O_RDONLY 0
#    define O_WRONLY 1
#    define O_RDWR   2
#    define O_CREAT  0x0200
#    define O_TRUNC  0x0400
#    define O_APPEND 0x0008
typedef int mode_t;

#else
#    include <sys/fcntl.h>
#endif

#include "platform.h"

#include "FileHandle.h"
#include "DirHandle.h"

namespace mbed {

/** A filesystem-like object is one that can be used to open files
 *  though it by fopen("/name/filename", mode)
 *
 *  Implementations must define at least open (the default definitions
 *  of the rest of the functions just return error values).
 */
class FileSystemLike {

public:
    /** FileSystemLike constructor
     *
     *  @param name The name to use for the filesystem.
     */
    FileSystemLike(const char *name);
    
    virtual ~FileSystemLike();
    
    /* Function lookup
     *  Lookup and return the object that has the given name.
     *
     * Variables
     *  name - the name to lookup.
     *  len - the length of name.
     */
    static FileSystemLike *lookup(const char *name, unsigned int len);
    
    static DirHandle *opendir();
    friend class BaseDirHandle;
    
    /** Opens a file from the filesystem
     *
     *  @param filename The name of the file to open.
     *  @param flags One of O_RDONLY, O_WRONLY, or O_RDWR, OR'd with
     *    zero or more of O_CREAT, O_TRUNC, or O_APPEND.
     *
     *  @returns
     *    A pointer to a FileHandle object representing the
     *   file on success, or NULL on failure.
     */
    virtual FileHandle *open(const char *filename, int flags) = 0;

    /** Remove a file from the filesystem.
     *
     *  @param filename the name of the file to remove.
     *  @param returns 0 on success, -1 on failure.
     */
    virtual int remove(const char *filename) { return -1; };

    /** Rename a file in the filesystem.
     *
     *  @param oldname the name of the file to rename.
     *  @param newname the name to rename it to.
     *
     *  @returns
     *    0 on success,
     *   -1 on failure.
     */
    virtual int rename(const char *oldname, const char *newname) { return -1; };

    /** Opens a directory in the filesystem and returns a DirHandle
     *   representing the directory stream.
     *
     *  @param name The name of the directory to open.
     *
     *  @returns
     *    A DirHandle representing the directory stream, or
     *   NULL on failure.
     */
    virtual DirHandle *opendir(const char *name) { return NULL; };

    /** Creates a directory in the filesystem.
     *
     *  @param name The name of the directory to create.
     *  @param mode The permissions to create the directory with.
     *
     *  @returns
     *    0 on success,
     *   -1 on failure.
     */
    virtual int mkdir(const char *name, mode_t mode) { return -1; }

    // TODO other filesystem functions (mkdir, rm, rn, ls etc)

protected: 
    static FileSystemLike *_head;
    FileSystemLike *_next;
    const char *_name;
};

class FilePath {
public:
    FilePath(const char* file_path);
    
    const char* fileName(void);
    FileSystemLike* fileSystem(void);

private:
    const char* file_name;
    FileSystemLike* fs;
};

} // namespace mbed

#endif
