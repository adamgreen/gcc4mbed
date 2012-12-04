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
#ifndef MBED_TIMEREVENT_H
#define MBED_TIMEREVENT_H

#include "us_ticker_api.h"

namespace mbed {

/** Base abstraction for timer interrupts
*/
class TimerEvent {
public:
    TimerEvent();
    
    /** The handler registered with the underlying timer interrupt
     */
    static void irq(uint32_t id);
    
    /** Destruction removes it...
     */
    virtual ~TimerEvent();

protected:
    // The handler called to service the timer event of the derived class
    virtual void handler() = 0;
    
    // insert in to linked list
    void insert(unsigned int timestamp);
    
    // remove from linked list, if in it
    void remove();
    
    ticker_event_t event;
};

} // namespace mbed

#endif
