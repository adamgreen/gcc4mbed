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

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Implementation of the C time.h functions
 *
 * Provides mechanisms to set and read the current time, based
 * on the microcontroller Real-Time Clock (RTC), plus some 
 * standard C manipulation and formating functions. 
 *
 * Example:
 * @code
 * #include "mbed.h"
 *
 * int main() {
 *     set_time(1256729737);  // Set RTC time to Wed, 28 Oct 2009 11:35:37
 *      
 *     while(1) {    
 *         time_t seconds = time(NULL);
 *         
 *         printf("Time as seconds since January 1, 1970 = %d\n", seconds);
 *  
 *         printf("Time as a basic string = %s", ctime(&seconds));
 *
 *         char buffer[32];
 *         strftime(buffer, 32, "%I:%M %p\n", localtime(&seconds));
 *         printf("Time as a custom formatted string = %s", buffer);
 *    
 *         wait(1);
 *     }
 * }
 * @endcode
 */

/** Set the current time
 *
 * Initialises and sets the time of the microcontroller Real-Time Clock (RTC)
 * to the time represented by the number of seconds since January 1, 1970 
 * (the UNIX timestamp). 
 * 
 * @param t Number of seconds since January 1, 1970 (the UNIX timestamp) 
 *
 * Example:
 * @code
 * #include "mbed.h"
 *
 * int main() {
 *     set_time(1256729737); // Set time to Wed, 28 Oct 2009 11:35:37
 * }
 * @endcode
 */ 
void set_time(time_t t);

#ifdef __cplusplus
}
#endif
