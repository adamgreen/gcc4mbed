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
#ifndef MBED_ANALOGOUT_API_H
#define MBED_ANALOGOUT_API_H

#include "device.h"

#if DEVICE_ANALOGOUT

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dac_s dac_t;

void     analogout_init     (dac_t *obj, PinName pin);
void     analogout_free     (dac_t *obj);
void     analogout_write    (dac_t *obj, float value);
void     analogout_write_u16(dac_t *obj, uint16_t value);
float    analogout_read     (dac_t *obj);
uint16_t analogout_read_u16 (dac_t *obj);

#ifdef __cplusplus
}
#endif

#endif

#endif
