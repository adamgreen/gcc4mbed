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
#ifndef MBED_INTERFACE_H
#define MBED_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

/** Functions to control the mbed interface
 *
 * mbed Microcontrollers have a built-in interface to provide functionality such as 
 * drag-n-drop download, reset, serial-over-usb, and access to the mbed local file 
 * system. These functions provide means to control the interface suing semihost
 * calls it supports.
 */

/** Determine whether the mbed interface is connected, based on whether debug is enabled
 * 
 *  @returns
 *    1 if interface is connected,
 *    0 otherwise
 */
int mbed_interface_connected(void);

/** Instruct the mbed interface to reset, as if the reset button had been pressed
 *
 *  @returns
 *    1 if successful,
 *    0 otherwise (e.g. interface not present)
 */
int mbed_interface_reset(void);

/** This will disconnect the debug aspect of the interface, so semihosting will be disabled.
 * The interface will still support the USB serial aspect
 *
 *  @returns
 *    0 if successful,
 *   -1 otherwise (e.g. interface not present)
 */
int mbed_interface_disconnect(void);

/** This will disconnect the debug aspect of the interface, and if the USB cable is not 
 * connected, also power down the interface. If the USB cable is connected, the interface
 * will remain powered up and visible to the host 
 *
 *  @returns
 *    0 if successful,
 *   -1 otherwise (e.g. interface not present)
 */
int mbed_interface_powerdown(void);

/** This returns a string containing the 32-character UID of the mbed interface
 *  This is a weak function that can be overwritten if required
 *
 *  @param uid A 33-byte array to write the null terminated 32-byte string
 *
 *  @returns
 *    0 if successful,
 *   -1 otherwise (e.g. interface not present)
 */
int mbed_interface_uid(char *uid);

/** This returns a unique 6-byte MAC address, based on the interface UID
 * If the interface is not present, it returns a default fixed MAC address (00:02:F7:F0:00:00)
 *
 * This is a weak function that can be overwritten if you want to provide your own mechanism to
 * provide a MAC address.
 *
 *  @param mac A 6-byte array to write the MAC address
 */
void mbed_mac_address(char *mac);

/** Cause the mbed to flash the BLOD LED sequence
 */
void mbed_die(void);

#ifdef __cplusplus
}
#endif

#endif
