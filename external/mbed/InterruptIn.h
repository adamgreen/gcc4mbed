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
#ifndef MBED_INTERRUPTIN_H
#define MBED_INTERRUPTIN_H

#include "platform.h"

#if DEVICE_INTERRUPTIN

#include "gpio_api.h"
#include "gpio_irq_api.h"

#include "FunctionPointer.h"

namespace mbed {

/** A digital interrupt input, used to call a function on a rising or falling edge
 *
 * Example:
 * @code
 * // Flash an LED while waiting for events
 *
 * #include "mbed.h"
 *
 * InterruptIn event(p16);
 * DigitalOut led(LED1);
 *
 * void trigger() {
 *     printf("triggered!\n");
 * }
 *
 * int main() {
 *     event.rise(&trigger);
 *     while(1) {
 *         led = !led;
 *         wait(0.25);
 *     }
 * }
 * @endcode
 */
class InterruptIn {

public:

    /** Create an InterruptIn connected to the specified pin
     *
     *  @param pin InterruptIn pin to connect to
     *  @param name (optional) A string to identify the object
     */
    InterruptIn(PinName pin);
    virtual ~InterruptIn();
    
     int read();
#ifdef MBED_OPERATORS
    operator int();

#endif
     
    /** Attach a function to call when a rising edge occurs on the input
     *
     *  @param fptr A pointer to a void function, or 0 to set as none
     */
    void rise(void (*fptr)(void));

    /** Attach a member function to call when a rising edge occurs on the input
     *     
     *  @param tptr pointer to the object to call the member function on
     *  @param mptr pointer to the member function to be called
     */
    template<typename T>
    void rise(T* tptr, void (T::*mptr)(void)) {
        _rise.attach(tptr, mptr);
        gpio_irq_set(&gpio_irq, IRQ_RISE, 1);
    }

    /** Attach a function to call when a falling edge occurs on the input
     *
     *  @param fptr A pointer to a void function, or 0 to set as none
     */
    void fall(void (*fptr)(void));

    /** Attach a member function to call when a falling edge occurs on the input
     *     
     *  @param tptr pointer to the object to call the member function on
     *  @param mptr pointer to the member function to be called
     */
    template<typename T>
    void fall(T* tptr, void (T::*mptr)(void)) {
        _fall.attach(tptr, mptr);
        gpio_irq_set(&gpio_irq, IRQ_FALL, 1);
    }

    /** Set the input pin mode
     *
     *  @param mode PullUp, PullDown, PullNone
     */
    void mode(PinMode pull);
    
    static void _irq_handler(uint32_t id, gpio_irq_event event);
    
protected:
    gpio_t gpio;
    gpio_irq_t gpio_irq;
    
    FunctionPointer _rise;
    FunctionPointer _fall;
};

} // namespace mbed

#endif

#endif
