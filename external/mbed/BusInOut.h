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
#ifndef MBED_BUSINOUT_H
#define MBED_BUSINOUT_H

#include "DigitalInOut.h"

namespace mbed {

/** A digital input output bus, used for setting the state of a collection of pins
 */
class BusInOut {

public:
    
    /** Create an BusInOut, connected to the specified pins
     *
     *  @param p<n> DigitalInOut pin to connect to bus bit p<n> (p5-p30, NC)
     *
     *  @note
     *  It is only required to specify as many pin variables as is required
     *  for the bus; the rest will default to NC (not connected)
     */
    BusInOut(PinName p0, PinName p1 = NC, PinName p2 = NC, PinName p3 = NC,
             PinName p4 = NC, PinName p5 = NC, PinName p6 = NC, PinName p7 = NC,
             PinName p8 = NC, PinName p9 = NC, PinName p10 = NC, PinName p11 = NC,
             PinName p12 = NC, PinName p13 = NC, PinName p14 = NC, PinName p15 = NC);

    BusInOut(PinName pins[16]);

    virtual ~BusInOut();

    /* Group: Access Methods */

    /** Write the value to the output bus
     *
     *  @param value An integer specifying a bit to write for every corresponding DigitalInOut pin
     */
    void write(int value);

        
    /** Read the value currently output on the bus
     *
     *  @returns
     *    An integer with each bit corresponding to associated DigitalInOut pin setting
     */
    int read();

    /** Set as an output
     */
    void output();

    /** Set as an input
     */
    void input();

    /** Set the input pin mode
     *
     *  @param mode PullUp, PullDown, PullNone
     */
    void mode(PinMode pull);
 
#ifdef MBED_OPERATORS
     /** A shorthand for write()
     */
    BusInOut& operator= (int v);
    BusInOut& operator= (BusInOut& rhs);

    /** A shorthand for read()
     */
    operator int();
#endif

protected:
    DigitalInOut* _pin[16];
};

} // namespace mbed

#endif
