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
#ifndef MBED_ETHERNET_H
#define MBED_ETHERNET_H

#include "platform.h"

#if DEVICE_ETHERNET

namespace mbed {

/** An ethernet interface, to use with the ethernet pins.
 *
 * Example:
 * @code
 * // Read destination and source from every ethernet packet
 * 
 * #include "mbed.h"
 *
 * Ethernet eth;
 *  
 * int main() {
 *     char buf[0x600];
 *     
 *     while(1) {
 *         int size = eth.receive();
 *         if(size > 0) {
 *             eth.read(buf, size);
 *             printf("Destination:  %02X:%02X:%02X:%02X:%02X:%02X\n",
 *                     buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
 *             printf("Source: %02X:%02X:%02X:%02X:%02X:%02X\n",
 *                     buf[6], buf[7], buf[8], buf[9], buf[10], buf[11]);
 *         }
 *         
 *         wait(1);
 *     }
 * }
 * @endcode
 */
class Ethernet {

public:
    
    /** Initialise the ethernet interface.
     */
    Ethernet();

    /** Powers the hardware down.
     */
    virtual ~Ethernet();

    enum Mode {
        AutoNegotiate,
        HalfDuplex10,
        FullDuplex10,
        HalfDuplex100,
        FullDuplex100
    };

    /** Writes into an outgoing ethernet packet.
     *
     *  It will append size bytes of data to the previously written bytes.
     *  
     *  @param data An array to write.
     *  @param size The size of data.
     *
     *  @returns
     *   The number of written bytes.
     */
    int write(const char *data, int size);

    /** Send an outgoing ethernet packet.
     *
     *  After filling in the data in an ethernet packet it must be send.
     *  Send will provide a new packet to write to.
     *
     *  @returns
     *    0 if the sending was failed,
     *    1 if the package is successfully sent.
     */
    int send();

    /** Recevies an arrived ethernet packet.
     *
     *  Receiving an ethernet packet will drop the last received ethernet packet 
     *  and make a new ethernet packet ready to read.
     *  If no ethernet packet is arrived it will return 0.
     *
     *  @returns
     *    0 if no ethernet packet is arrived,
     *    or the size of the arrived packet.
     */
    int receive();

    /** Read from an recevied ethernet packet.
     *
     *  After receive returnd a number bigger than 0it is
     *  possible to read bytes from this packet.
     *  Read will write up to size bytes into data.
     *
     *  It is possible to use read multible times. 
     *  Each time read will start reading after the last read byte before.
     *
     *  @returns
     *  The number of byte read.
     */
    int read(char *data, int size);
    
    /** Gives the ethernet address of the mbed.
     *
     *  @param mac Must be a pointer to a 6 byte char array to copy the ethernet address in.
     */
    void address(char *mac);

    /** Returns if an ethernet link is pressent or not. It takes a wile after Ethernet initializion to show up.
     * 
     *  @returns
     *   0 if no ethernet link is pressent,
     *   1 if an ethernet link is pressent.
     *
     * Example:
     * @code
     * // Using the Ethernet link function
     * #include "mbed.h"
     * 
     * Ethernet eth;
     *
     * int main() {
     *     wait(1); // Needed after startup.
     *     if (eth.link()) {
     *          printf("online\n");
     *     } else {
     *          printf("offline\n");
     *     }
     * }
     * @endcode
     */
    int link();

    /** Sets the speed and duplex parameters of an ethernet link
     *
     * - AutoNegotiate      Auto negotiate speed and duplex
     * - HalfDuplex10       10 Mbit, half duplex
     * - FullDuplex10       10 Mbit, full duplex
     * - HalfDuplex100      100 Mbit, half duplex
     * - FullDuplex100      100 Mbit, full duplex
     *
     *  @param mode the speed and duplex mode to set the link to:
     */
    void set_link(Mode mode);
};

} // namespace mbed

#endif

#endif
