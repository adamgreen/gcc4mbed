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
#ifndef MBED_CAN_API_H
#define MBED_CAN_API_H

#include "device.h"

#if DEVICE_CAN

#include "PinNames.h" 
#include "PeripheralNames.h"
#include "can_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct can_s can_t;

void          can_init     (can_t *obj, PinName rd, PinName td);
void          can_free     (can_t *obj);
int           can_frequency(can_t *obj, int hz);
int           can_write    (can_t *obj, CAN_Message, int cc);
int           can_read     (can_t *obj, CAN_Message *msg);
void          can_reset    (can_t *obj);
unsigned char can_rderror  (can_t *obj);
unsigned char can_tderror  (can_t *obj);
void          can_monitor  (can_t *obj, int silent);

#ifdef __cplusplus
};
#endif

#endif    // MBED_CAN_API_H

#endif
