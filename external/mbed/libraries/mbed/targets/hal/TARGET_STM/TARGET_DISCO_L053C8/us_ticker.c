/* mbed Microcontroller Library
 * Copyright (c) 2014, STMicroelectronics
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stddef.h>
#include "us_ticker_api.h"
#include "PeripheralNames.h"

// Timer selection:
#define TIM_MST      TIM21
#define TIM_MST_IRQ  TIM21_IRQn
#define TIM_MST_RCC  __TIM21_CLK_ENABLE()

static TIM_HandleTypeDef TimMasterHandle;

static int      us_ticker_inited = 0;
static volatile uint32_t SlaveCounter = 0;
static volatile uint32_t oc_int_part = 0;
static volatile uint16_t oc_rem_part = 0;

void set_compare(uint16_t count) {
    // Set new output compare value
    __HAL_TIM_SetCompare(&TimMasterHandle, TIM_CHANNEL_1, count);
    // Enable IT
    __HAL_TIM_ENABLE_IT(&TimMasterHandle, TIM_IT_CC1);
}

static void tim_irq_handler(void) {
    uint16_t cval = TIM_MST->CNT;

    // Clear Update interrupt flag
    if (__HAL_TIM_GET_FLAG(&TimMasterHandle, TIM_FLAG_UPDATE) == SET) {
        __HAL_TIM_CLEAR_FLAG(&TimMasterHandle, TIM_FLAG_UPDATE);
        SlaveCounter++;
    }

    // Clear CC1 interrupt flag
    if (__HAL_TIM_GET_FLAG(&TimMasterHandle, TIM_FLAG_CC1) == SET) {
        __HAL_TIM_CLEAR_FLAG(&TimMasterHandle, TIM_FLAG_CC1);
        if (oc_rem_part > 0) {
            set_compare(oc_rem_part); // Finish the remaining time left
            oc_rem_part = 0;
        } else {
            if (oc_int_part > 0) {
                set_compare(0xFFFF);
                oc_rem_part = cval; // To finish the counter loop the next time
                oc_int_part--;
            } else {
                us_ticker_irq_handler();
            }
        }
    }
}

void us_ticker_init(void) {
    if (us_ticker_inited) return;
    us_ticker_inited = 1;

    // Enable timer clock
    TIM_MST_RCC;

    // Configure time base
    TimMasterHandle.Instance = TIM_MST;
    TimMasterHandle.Init.Period        = 0xFFFF;
    TimMasterHandle.Init.Prescaler     = (uint32_t)(SystemCoreClock / 1000000) - 1; // 1 �s tick
    TimMasterHandle.Init.ClockDivision = 0;
    TimMasterHandle.Init.CounterMode   = TIM_COUNTERMODE_UP;
    HAL_TIM_Base_Init(&TimMasterHandle);

    // Configure interrupts
    __HAL_TIM_ENABLE_IT(&TimMasterHandle, TIM_IT_UPDATE);

    // Update interrupt used for 32-bit counter
    // Output compare interrupt used for timeout feature
    NVIC_SetVector(TIM_MST_IRQ, (uint32_t)tim_irq_handler);
    NVIC_EnableIRQ(TIM_MST_IRQ);

    // Enable timer
    HAL_TIM_Base_Start(&TimMasterHandle);
}

uint32_t us_ticker_read() {
    uint32_t counter, counter2;
    if (!us_ticker_inited) us_ticker_init();
    // A situation might appear when Master overflows right after Slave is read and before the
    // new (overflowed) value of Master is read. Which would make the code below consider the
    // previous (incorrect) value of Slave and the new value of Master, which would return a
    // value in the past. Avoid this by computing consecutive values of the timer until they
    // are properly ordered.
    counter = (uint32_t)(SlaveCounter << 16);
    counter += TIM_MST->CNT;
    while (1) {
        counter2 = (uint32_t)(SlaveCounter << 16);
        counter2 += TIM_MST->CNT;
        if (counter2 > counter) {
            break;
        }
        counter = counter2;
    }
    return counter2;
}

void us_ticker_set_interrupt(timestamp_t timestamp) {
    int delta = (int)((uint32_t)timestamp - us_ticker_read());
    uint16_t cval = TIM_MST->CNT;

    if (delta <= 0) { // This event was in the past
        us_ticker_irq_handler();
    } else {
        oc_int_part = (uint32_t)(delta >> 16);
        oc_rem_part = (uint16_t)(delta & 0xFFFF);
        if (oc_rem_part <= (0xFFFF - cval)) {
            set_compare(cval + oc_rem_part);
            oc_rem_part = 0;
        } else {
            set_compare(0xFFFF);
            oc_rem_part = oc_rem_part - (0xFFFF - cval);
        }
    }
}

void us_ticker_disable_interrupt(void) {
    __HAL_TIM_DISABLE_IT(&TimMasterHandle, TIM_IT_CC1);
}

void us_ticker_clear_interrupt(void) {
    __HAL_TIM_CLEAR_FLAG(&TimMasterHandle, TIM_FLAG_CC1);
}
