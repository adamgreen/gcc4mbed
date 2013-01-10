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
#ifndef MBED_FILELIKE_H
#define MBED_FILELIKE_H

#include "FileBase.h"
#include "FileHandle.h"

namespace mbed {

/* Class FileLike
 *  A file-like object is one that can be opened with fopen by
 *  fopen("/name", mode). It is intersection of the classes Base and
 *  FileHandle.
 */
class FileLike : public FileHandle, public FileBase {

public:
    /* Constructor FileLike
     *
     * Variables
     *  name - The name to use to open the file.
     */
    FileLike(const char *name);
    
    virtual ~FileLike();
};

} // namespace mbed

#endif
