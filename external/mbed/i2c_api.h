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
#ifndef MBED_I2C_API_H
#define MBED_I2C_API_H

#include "device.h"

#if DEVICE_I2C

#ifdef __cplusplus
extern "C" {
#endif

typedef struct i2c_s i2c_t;

void i2c_init         (i2c_t *obj, PinName sda, PinName scl);
void i2c_frequency    (i2c_t *obj, int hz);
int  i2c_start        (i2c_t *obj);
void i2c_stop         (i2c_t *obj);
int  i2c_read         (i2c_t *obj, int address, char *data, int length, int stop); 
int  i2c_write        (i2c_t *obj, int address, const char *data, int length, int stop);
void i2c_reset        (i2c_t *obj);
int  i2c_byte_read    (i2c_t *obj, int last);
int  i2c_byte_write   (i2c_t *obj, int data);

#if DEVICE_I2CSLAVE
void i2c_slave_mode   (i2c_t *obj, int enable_slave);
int  i2c_slave_receive(i2c_t *obj);
int  i2c_slave_read   (i2c_t *obj, char *data, int length);
int  i2c_slave_write  (i2c_t *obj, const char *data, int length);
void i2c_slave_address(i2c_t *obj, int idx, uint32_t address, uint32_t mask);
#endif

#ifdef __cplusplus
}
#endif

#endif

#endif
