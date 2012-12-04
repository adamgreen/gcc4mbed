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
#ifndef MBED_STREAM_H
#define MBED_STREAM_H

#include "platform.h"
#include "FileHandle.h"

namespace mbed {

class Stream : public FileHandle {

public:
    Stream(void);
    virtual ~Stream();

    int putc(int c);
    int puts(const char *s);
    int getc();
    char *gets(char *s, int size);
    int printf(const char* format, ...);
    int scanf(const char* format, ...);
    
    operator std::FILE*() {return _file;}

protected:
    virtual int close();
    virtual ssize_t write(const void* buffer, size_t length);
    virtual ssize_t read(void* buffer, size_t length);
    virtual off_t lseek(off_t offset, int whence);
    virtual int isatty();
    virtual int fsync();
    virtual off_t flen();

    virtual int _putc(int c) = 0;
    virtual int _getc() = 0;
    
    std::FILE *_file;
};

} // namespace mbed

#endif
