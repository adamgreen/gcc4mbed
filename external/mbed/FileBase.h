/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
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
#ifndef MBED_FILEBASE_H
#define MBED_FILEBASE_H

typedef int FILEHANDLE;

#include <stdio.h>

#ifdef __ARMCC_VERSION
#    define O_RDONLY 0
#    define O_WRONLY 1
#    define O_RDWR   2
#    define O_CREAT  0x0200
#    define O_TRUNC  0x0400
#    define O_APPEND 0x0008

#    define NAME_MAX 255

typedef int mode_t;
typedef int ssize_t;
typedef long off_t;

#else
#    include <sys/fcntl.h>
#    include <sys/types.h>
#    include <sys/syslimits.h>
#endif

#include "platform.h"

namespace mbed {

typedef enum {
    FilePathType,
    FileSystemPathType
} PathType;

class FileBase {
public:
    FileBase(const char *name, PathType t);
    
    virtual ~FileBase();
    
    const char* getName(void);
    PathType    getPathType(void);
    
    static FileBase *lookup(const char *name, unsigned int len);
    
    static FileBase *get(int n);

protected: 
    static FileBase *_head;
    
    FileBase   *_next;
    const char *_name;
    PathType    _path_type;
};

} // namespace mbed

#endif
