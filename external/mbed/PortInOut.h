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
#ifndef MBED_PORTINOUT_H
#define MBED_PORTINOUT_H

#include "platform.h"

#if DEVICE_PORTINOUT

#include "port_api.h"

namespace mbed {

/** A multiple pin digital in/out used to set/read multiple bi-directional pins
 */
class PortInOut {
public:

    /** Create an PortInOut, connected to the specified port
     *
     *  @param port Port to connect to (Port0-Port5)
     *  @param mask A bitmask to identify which bits in the port should be included (0 - ignore)
     */ 
    PortInOut(PortName port, int mask = 0xFFFFFFFF) {
        port_init(&_port, port, mask, PIN_INPUT);
    }
    
    /** Write the value to the output port
     *
     *  @param value An integer specifying a bit to write for every corresponding port pin
     */    
    void write(int value) {
        port_write(&_port, value);
    }
    
    /** Read the value currently output on the port
     *
     *  @returns
     *    An integer with each bit corresponding to associated port pin setting
     */
    int read() {
        return port_read(&_port);
    }
    
    /** Set as an output
     */
    void output() {
        port_dir(&_port, PIN_OUTPUT);
    }
    
    /** Set as an input
     */
    void input() {
        port_dir(&_port, PIN_INPUT);
    }
    
    /** Set the input pin mode
     *
     *  @param mode PullUp, PullDown, PullNone, OpenDrain
     */
    void mode(PinMode mode) {
        port_mode(&_port, mode);
    }
    
    /** A shorthand for write()
     */    
    PortInOut& operator= (int value) {
        write(value);
        return *this;
    }
    
    PortInOut& operator= (PortInOut& rhs) {
        write(rhs.read());
        return *this;
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
