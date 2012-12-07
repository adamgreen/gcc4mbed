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
#ifndef MBED_SERIAL_H
#define MBED_SERIAL_H

#include "platform.h"

#if DEVICE_SERIAL

#include "Stream.h"
#include "FunctionPointer.h"
#include "serial_api.h"

namespace mbed {

/** A serial port (UART) for communication with other serial devices
 *
 * Can be used for Full Duplex communication, or Simplex by specifying 
 * one pin as NC (Not Connected)
 *
 * Example:
 * @code
 * // Print "Hello World" to the PC
 *
 * #include "mbed.h"
 *
 * Serial pc(USBTX, USBRX);
 *
 * int main() {
 *     pc.printf("Hello World\n");
 * }
 * @endcode
 */
class Serial : public Stream {

public:

    /** Create a Serial port, connected to the specified transmit and receive pins
     *
     *  @param tx Transmit pin 
     *  @param rx Receive pin
     *
     *  @note
     *    Either tx or rx may be specified as NC if unused
     */
    Serial(PinName tx, PinName rx) {
        serial_init(&_serial, tx, rx);
        serial_irq_handler(&_serial, Serial::_irq_handler, (uint32_t)this);
    }

    /** Set the baud rate of the serial port
     *  
     *  @param baudrate The baudrate of the serial port (default = 9600).
     */
    void baud(int baudrate) {
        serial_baud(&_serial, baudrate);
    }
    
    enum Parity {
        None = 0,
        Odd,
        Even,
        Forced1,
        Forced0
    };
    
    enum IrqType {
        RxIrq = 0,
        TxIrq
    };

    /** Set the transmission format used by the Serial port
     *
     *  @param bits The number of bits in a word (5-8; default = 8)
     *  @param parity The parity used (Serial::None, Serial::Odd, Serial::Even, Serial::Forced1, Serial::Forced0; default = Serial::None)
     *  @param stop The number of stop bits (1 or 2; default = 1)
     */
    void format(int bits = 8, Parity parity=Serial::None, int stop_bits=1) {
        serial_format(&_serial, bits, (SerialParity)parity, stop_bits);
    }

    /** Determine if there is a character available to read
     *
     *  @returns
     *    1 if there is a character available to read,
     *    0 otherwise
     */
    int readable() {
        return serial_readable(&_serial);
    }

    /** Determine if there is space available to write a character
     * 
     *  @returns
     *    1 if there is space to write a character,
     *    0 otherwise
     */
    int writeable() {
        return serial_writable(&_serial);
    }

    /** Attach a function to call whenever a serial interrupt is generated
     *
     *  @param fptr A pointer to a void function, or 0 to set as none
     *  @param type Which serial interrupt to attach the member function to (Seriall::RxIrq for receive, TxIrq for transmit buffer empty)
     */
    void attach(void (*fptr)(void), IrqType type=RxIrq) {
        if (fptr) {
            _irq[type].attach(fptr);
            serial_irq_set(&_serial, (SerialIrq)type, 1);
        } else {
            serial_irq_set(&_serial, (SerialIrq)type, 0);
        }
    }

    /** Attach a member function to call whenever a serial interrupt is generated
     *     
     *  @param tptr pointer to the object to call the member function on
     *  @param mptr pointer to the member function to be called
     *  @param type Which serial interrupt to attach the member function to (Seriall::RxIrq for receive, TxIrq for transmit buffer empty)
     */
    template<typename T>
    void attach(T* tptr, void (T::*mptr)(void), IrqType type=RxIrq) {
        if((mptr != NULL) && (tptr != NULL)) {
            _irq[type].attach(tptr, mptr);
            serial_irq_set(&_serial, (SerialIrq)type, 1);
        }
    }
    
    static void _irq_handler(uint32_t id, SerialIrq irq_type) {
        Serial *handler = (Serial*)id;
        handler->_irq[irq_type].call();
    }

protected:
    virtual int _getc() {
        return serial_getc(&_serial);
    }
    
    virtual int _putc(int c) {
        serial_putc(&_serial, c);
        return c;
    }
    
    serial_t   _serial;
    FunctionPointer _irq[2];
};

} // namespace mbed

#endif

#endif
