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
#ifndef MBED_DIRHANDLE_H
#define MBED_DIRHANDLE_H

#ifdef __ARMCC_VERSION
# define NAME_MAX 255
typedef int mode_t;
#else
# include <sys/syslimits.h>
#endif
#include "FileHandle.h"

struct dirent {
    char d_name[NAME_MAX+1];
};

namespace mbed {

/** Represents a directory stream. Objects of this type are returned
 *  by a FileSystemLike's opendir method. Implementations must define
 *  at least closedir, readdir and rewinddir.
 *
 *  If a FileSystemLike class defines the opendir method, then the
 *  directories of an object of that type can be accessed by 
 *  DIR *d = opendir("/example/directory") (or opendir("/example")
 *  to open the root of the filesystem), and then using readdir(d) etc.
 *
 *  The root directory is considered to contain all FileLike and
 *  FileSystemLike objects, so the DIR* returned by opendir("/") will
 *  reflect this.
 */
class DirHandle {

public:
    /** Closes the directory.
     *
     *  @returns
     *    0 on success,
     *   -1 on error.
     */
    virtual int closedir()=0;
    
    /** Return the directory entry at the current position, and
     *  advances the position to the next entry.
     *
     * @returns
     *  A pointer to a dirent structure representing the
     *  directory entry at the current position, or NULL on reaching
     *  end of directory or error.
     */
    virtual struct dirent *readdir()=0;

    /** Resets the position to the beginning of the directory.
     */
    virtual void rewinddir()=0;

    /** Returns the current position of the DirHandle.
     *
     * @returns
     *   the current position,
     *  -1 on error.
     */
    virtual off_t telldir() { return -1; }

    /** Sets the position of the DirHandle.
     *
     *  @param location The location to seek to. Must be a value returned by telldir.
     */
    virtual void seekdir(off_t location) { }
    
    virtual ~DirHandle() {}
};

} // namespace mbed

typedef mbed::DirHandle DIR;

extern "C" {
    DIR *opendir(const char*);
    struct dirent *readdir(DIR *);
    int closedir(DIR*);
    void rewinddir(DIR*);
    long telldir(DIR*);
    void seekdir(DIR*, long);
    int mkdir(const char *name, mode_t n);
};

#endif /* MBED_DIRHANDLE_H */
