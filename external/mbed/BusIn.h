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
#ifndef MBED_BUSIN_H
#define MBED_BUSIN_H

#include "platform.h"
#include "DigitalIn.h"

namespace mbed {

/** A digital input bus, used for reading the state of a collection of pins
 */
class BusIn {

public:
    /* Group: Configuration Methods */

    /** Create an BusIn, connected to the specified pins
     *
     * @param <n> DigitalIn pin to connect to bus bit <n> (p5-p30, NC)
     *
     * @note
     *  It is only required to specify as many pin variables as is required
     *  for the bus; the rest will default to NC (not connected)
     */ 
    BusIn(PinName p0, PinName p1 = NC, PinName p2 = NC, PinName p3 = NC,
          PinName p4 = NC, PinName p5 = NC, PinName p6 = NC, PinName p7 = NC,
          PinName p8 = NC, PinName p9 = NC, PinName p10 = NC, PinName p11 = NC,
          PinName p12 = NC, PinName p13 = NC, PinName p14 = NC, PinName p15 = NC);

    BusIn(PinName pins[16]);
    
    virtual ~BusIn();
    
    /** Read the value of the input bus
     *
     *  @returns
     *   An integer with each bit corresponding to the value read from the associated DigitalIn pin
     */
    int read();

#ifdef MBED_OPERATORS
    /** A shorthand for read()
     */
    operator int();
#endif

protected:
    DigitalIn* _pin[16];
};

} // namespace mbed

#endif
