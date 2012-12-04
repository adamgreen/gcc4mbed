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
#ifndef MBED_TIMER_H
#define MBED_TIMER_H

#include "platform.h"

namespace mbed {

/** A general purpose timer 
 *
 * Example:
 * @code
 * // Count the time to toggle a LED
 *
 * #include "mbed.h"
 * 
 * Timer timer;
 * DigitalOut led(LED1);
 * int begin, end;
 * 
 * int main() {
 *     timer.start();
 *     begin = timer.read_us();
 *     led = !led;
 *     end = timer.read_us();
 *     printf("Toggle the led takes %d us", end - begin);
 * }
 * @endcode
 */
class Timer {

public:
    Timer();
    
    /** Start the timer
     */
    void start(); 

    /** Stop the timer
     */
    void stop(); 

    /** Reset the timer to 0. 
     *
     * If it was already counting, it will continue
     */
    void reset();

    /** Get the time passed in seconds
     */
    float read();

    /** Get the time passed in mili-seconds
     */
    int read_ms();

    /** Get the time passed in micro-seconds
     */
    int read_us();

#ifdef MBED_OPERATORS 
    operator float();
#endif

protected:
    int slicetime();
    int _running;          // whether the timer is running
    unsigned int _start;   // the start time of the latest slice
    int _time;             // any accumulated time from previous slices
};

} // namespace mbed

#endif
