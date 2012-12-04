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
#ifndef MBED_I2C_SLAVE_H
#define MBED_I2C_SLAVE_H

#include "platform.h"

#if DEVICE_I2CSLAVE

#include "i2c_api.h"

namespace mbed {

/** An I2C Slave, used for communicating with an I2C Master device
 *
 * Example:
 * @code
 * // Simple I2C responder
 * #include <mbed.h>
 *
 * I2CSlave slave(p9, p10);
 *
 * int main() {
 *     char buf[10];
 *     char msg[] = "Slave!";
 *
 *     slave.address(0xA0);
 *     while (1) {
 *         int i = slave.receive();
 *         switch (i) {
 *             case I2CSlave::ReadAddressed: 
 *                 slave.write(msg, strlen(msg) + 1); // Includes null char
 *                 break;
 *             case I2CSlave::WriteGeneral:
 *                 slave.read(buf, 10);
 *                 printf("Read G: %s\n", buf);
 *                 break;
 *             case I2CSlave::WriteAddressed:
 *                 slave.read(buf, 10);
 *                 printf("Read A: %s\n", buf);
 *                 break;
 *         }
 *         for(int i = 0; i < 10; i++) buf[i] = 0;    // Clear buffer
 *     }
 * }
 * @endcode                  
 */
class I2CSlave {

public:
    enum RxStatus {
        NoData         = 0,
        ReadAddressed  = 1,
        WriteGeneral   = 2,
        WriteAddressed = 3
    };

    /** Create an I2C Slave interface, connected to the specified pins.
     *
     *  @param sda I2C data line pin
     *  @param scl I2C clock line pin
     */
    I2CSlave(PinName sda, PinName scl);

    /** Set the frequency of the I2C interface
     *
     *  @param hz The bus frequency in hertz
     */
    void frequency(int hz);

    /** Checks to see if this I2C Slave has been addressed.
     *
     *  @returns
     *  A status indicating if the device has been addressed, and how
     *  - NoData            - the slave has not been addressed
     *  - ReadAddressed     - the master has requested a read from this slave
     *  - WriteAddressed    - the master is writing to this slave
     *  - WriteGeneral      - the master is writing to all slave 
     */
    int receive(void);

    /** Read from an I2C master.
     *
     *  @param data pointer to the byte array to read data in to
     *  @param length maximum number of bytes to read
     *
     *  @returns
     *       0 on success,
     *   non-0 otherwise
     */
    int read(char *data, int length); 

    /** Read a single byte from an I2C master.
     *
     *  @returns
     *    the byte read
     */
    int read(void);

    /** Write to an I2C master.
     *
     *  @param data pointer to the byte array to be transmitted
     *  @param length the number of bytes to transmite
     *
     *  @returns
     *       0 on success,
     *   non-0 otherwise
     */
    int write(const char *data, int length);

    /** Write a single byte to an I2C master.
     *
     *  @data the byte to write
     *
     *  @returns
     *    '1' if an ACK was received,
     *    '0' otherwise
     */
    int write(int data);

    /** Sets the I2C slave address.
     *
     *  @param address The address to set for the slave (ignoring the least
     *  signifcant bit). If set to 0, the slave will only respond to the
     *  general call address.
     */
    void address(int address);

    /** Reset the I2C slave back into the known ready receiving state.
     */
    void stop(void);

protected:
    i2c_t _i2c;
};

} // namespace mbed

#endif

#endif
