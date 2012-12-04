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
#ifndef MBED_PORTIN_H
#define MBED_PORTIN_H

#include "platform.h"

#if DEVICE_PORTIN

#include "port_api.h"

namespace mbed {

/** A multiple pin digital input
 *
 *  Example:
 * @code
 * // Switch on an LED if any of mbed pins 21-26 is high
 *
 * #include "mbed.h"
 *
 * PortIn     p(Port2, 0x0000003F);   // p21-p26
 * DigitalOut ind(LED4);
 *
 * int main() {
 *     while(1) {
 *         int pins = p.read();
 *         if(pins) {
 *             ind = 1;
 *         } else {
 *             ind = 0;
 *         }
 *     }
 * }
 * @endcode
 */
class PortIn {
public:

    /** Create an PortIn, connected to the specified port
     *
     *  @param port Port to connect to (Port0-Port5)
     *  @param mask A bitmask to identify which bits in the port should be included (0 - ignore)
        */ 
    PortIn(PortName port, int mask = 0xFFFFFFFF) {
        port_init(&_port, port, mask, PIN_INPUT);
    }
    
    /** Read the value currently output on the port
     *
     *  @returns
     *    An integer with each bit corresponding to associated port pin setting
     */
    int read() {
        return port_read(&_port);
    }
    
    /** Set the input pin mode
     *
     *  @param mode PullUp, PullDown, PullNone, OpenDrain
     */
    void mode(PinMode mode) {
        port_mode(&_port, mode);
    }
    
    /** A shorthand for read()
     */
    operator int() { 
        return read();
    }

private:
    port_t _port;
};

} // namespace mbed

#endif

#endif
