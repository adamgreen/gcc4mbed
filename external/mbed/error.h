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
#ifndef MBED_ERROR_H
#define MBED_ERROR_H

/** To generate a fatal compile-time error, you can use the pre-processor #error directive.
 *
 * @code
 * #error "That shouldn't have happened!"
 * @endcode
 *
 * If the compiler evaluates this line, it will report the error and stop the compile.
 *
 * For example, you could use this to check some user-defined compile-time variables:
 *
 * @code
 * #define NUM_PORTS 7
 * #if (NUM_PORTS > 4)
 *     #error "NUM_PORTS must be less than 4"
 * #endif
 * @endcode
 *
 * Reporting Run-Time Errors:
 * To generate a fatal run-time error, you can use the mbed error() function.
 *
 * @code
 * error("That shouldn't have happened!");
 * @endcode
 *
 * If the mbed running the program executes this function, it will print the 
 * message via the USB serial port, and then die with the blue lights of death!
 *
 * The message can use printf-style formatting, so you can report variables in the 
 * message too. For example, you could use this to check a run-time condition:
 * 
 * @code
 * if(x >= 5) {
 *     error("expected x to be less than 5, but got %d", x);
 * }
 * #endcode
 */

#include <stdlib.h>
#include "device.h"

#ifdef DEVICE_STDIO_MESSAGES
    #include <stdio.h>
    #define error(...) (fprintf(stderr, __VA_ARGS__), exit(1))
#else
    #define error(...) (exit(1))
#endif

#endif
