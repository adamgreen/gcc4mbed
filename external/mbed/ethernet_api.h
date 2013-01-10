/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
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
#ifndef MBED_ETHERNET_API_H
#define MBED_ETHERNET_API_H

#include "device.h"

#if DEVICE_ETHERNET

#ifdef __cplusplus
extern "C" {
#endif

// Connection constants

int ethernet_init(void);
void ethernet_free(void);

// write size bytes from data to ethernet buffer
// return num bytes written
// or -1 if size is too big
int ethernet_write(const char *data, int size);

// send ethernet write buffer, returning the packet size sent
int ethernet_send(void);

// recieve from ethernet buffer, returning packet size, or 0 if no packet
int ethernet_receive(void);

// read size bytes in to data, return actual num bytes read (0..size)
// if data == NULL, throw the bytes away
int ethernet_read(char *data, int size);

// get the ethernet address
void ethernet_address(char *mac);

// see if the link is up
int ethernet_link(void);

// force link settings
void ethernet_set_link(int speed, int duplex);

#ifdef __cplusplus
}
#endif

#endif

#endif

