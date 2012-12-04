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
#ifndef MBED_TICKER_H
#define MBED_TICKER_H

#include "TimerEvent.h"
#include "FunctionPointer.h"

namespace mbed {

/** A Ticker is used to call a function at a recurring interval
 *
 *  You can use as many seperate Ticker objects as you require. 
 *
 * Example:
 * @code
 * // Toggle the blinking led after 5 seconds
 *
 * #include "mbed.h"
 *
 * Ticker timer;
 * DigitalOut led1(LED1);
 * DigitalOut led2(LED2);
 * 
 * int flip = 0;
 * 
 * void attime() {
 *     flip = !flip;
 * }
 *
 * int main() {
 *     timer.attach(&attime, 5);
 *     while(1) {
 *         if(flip == 0) {
 *             led1 = !led1;
 *         } else {
 *             led2 = !led2;
 *         }
 *         wait(0.2);
 *     }
 * }
 * @endcode
 */
class Ticker : public TimerEvent {

public:

    /** Attach a function to be called by the Ticker, specifiying the interval in seconds
     *
     *  @param fptr pointer to the function to be called
     *  @param t the time between calls in seconds
     */
    void attach(void (*fptr)(void), float t) {
        attach_us(fptr, t * 1000000.0f);
    }
    
    /** Attach a member function to be called by the Ticker, specifiying the interval in seconds
     *
     *  @param tptr pointer to the object to call the member function on
     *  @param mptr pointer to the member function to be called
     *  @param t the time between calls in seconds
     */
    template<typename T>
    void attach(T* tptr, void (T::*mptr)(void), float t) {
        attach_us(tptr, mptr, t * 1000000.0f);
    }
    
    /** Attach a function to be called by the Ticker, specifiying the interval in micro-seconds
     *
     *  @param fptr pointer to the function to be called
     *  @param t the time between calls in micro-seconds
     */
    void attach_us(void (*fptr)(void), unsigned int t) {
        _function.attach(fptr);
        setup(t);
    }

    /** Attach a member function to be called by the Ticker, specifiying the interval in micro-seconds
     *
     *  @param tptr pointer to the object to call the member function on
     *  @param mptr pointer to the member function to be called
     *  @param t the time between calls in micro-seconds
     */    
    template<typename T>
    void attach_us(T* tptr, void (T::*mptr)(void), unsigned int t) {
        _function.attach(tptr, mptr);
        setup(t);
    }
    
    /** Detach the function
     */
    void detach();

protected:
    void setup(unsigned int t);
    virtual void handler();
    
    unsigned int _delay;
    FunctionPointer _function;
};

} // namespace mbed

#endif
