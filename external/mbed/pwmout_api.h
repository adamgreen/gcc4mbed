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
#ifndef MBED_PWMOUT_API_H
#define MBED_PWMOUT_API_H

#include "device.h"

#if DEVICE_PWMOUT

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct pwmout_s pwmout_t;

void pwmout_init         (pwmout_t* obj, PinName pin);
void pwmout_free         (pwmout_t* obj);

void  pwmout_write       (pwmout_t* obj, float percent);
float pwmout_read        (pwmout_t* obj);

void pwmout_period       (pwmout_t* obj, float seconds);
void pwmout_period_ms    (pwmout_t* obj, int ms);
void pwmout_period_us    (pwmout_t* obj, int us);

void pwmout_pulsewidth   (pwmout_t* obj, float seconds);
void pwmout_pulsewidth_ms(pwmout_t* obj, int ms);
void pwmout_pulsewidth_us(pwmout_t* obj, int us);

#ifdef __cplusplus
}
#endif

#endif

#endif
