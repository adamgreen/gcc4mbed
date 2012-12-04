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
#ifndef MBED_CAN_H
#define MBED_CAN_H

#include "platform.h"

#if DEVICE_CAN

#include "can_api.h"
#include "can_helper.h" 
#include "FunctionPointer.h"

namespace mbed {

/** CANMessage class
 */
class CANMessage : public CAN_Message {

public:
    /** Creates empty CAN message.
     */
    CANMessage() {
        len    = 8;
        type   = CANData;
        format = CANStandard;
        id     = 0;
        memset(data, 0, 8);
    }
    
    /** Creates CAN message with specific content.
     */
    CANMessage(int _id, const char *_data, char _len = 8, CANType _type = CANData, CANFormat _format = CANStandard) {
      len    = _len & 0xF;
      type   = _type;
      format = _format;
      id     = _id;
      memcpy(data, _data, _len);
    }

    /** Creates CAN remote message.
     */
    CANMessage(int _id, CANFormat _format = CANStandard) {
      len    = 0;
      type   = CANRemote;
      format = _format;
      id     = _id;
      memset(data, 0, 8);
    }
};

/** A can bus client, used for communicating with can devices
 */
class CAN {

public:
    /** Creates an CAN interface connected to specific pins.
     *
     *  @param rd read from transmitter
     *  @param td transmit to transmitter
     *
     * Example:
     * @code
     * #include "mbed.h"
     *  
     * Ticker ticker;
     * DigitalOut led1(LED1);
     * DigitalOut led2(LED2);
     * CAN can1(p9, p10);
     * CAN can2(p30, p29);
     *  
     * char counter = 0;
     *  
     * void send() {
     *     if(can1.write(CANMessage(1337, &counter, 1))) {
     *         printf("Message sent: %d\n", counter);
     *         counter++;
     *     } 
     *     led1 = !led1;
     * }
     *  
     * int main() {
     *     ticker.attach(&send, 1);
     *    CANMessage msg;
     *     while(1) {
     *         if(can2.read(msg)) {
     *             printf("Message received: %d\n\n", msg.data[0]);
     *             led2 = !led2;
     *         } 
     *         wait(0.2);
     *     }
     * } 
     * @endcode
     */
    CAN(PinName rd, PinName td);
    virtual ~CAN();
    
    /** Set the frequency of the CAN interface
     *
     *  @param hz The bus frequency in hertz
     *
     *  @returns
     *    1 if successful,
     *    0 otherwise
     */
    int frequency(int hz);
    
    /** Write a CANMessage to the bus.
     *
     *  @param msg The CANMessage to write.
     *
     *  @returns
     *    0 if write failed,
     *    1 if write was successful
     */
    int write(CANMessage msg);
    
    /** Read a CANMessage from the bus.
     * 
     *  @param msg A CANMessage to read to.
     *
     *  @returns
     *    0 if no message arrived,
     *    1 if message arrived
     */
    int read(CANMessage &msg);
    
    /** Reset CAN interface.
     *
     * To use after error overflow.
     */
    void reset();
    
    /** Puts or removes the CAN interface into silent monitoring mode
     *
     *  @param silent boolean indicating whether to go into silent mode or not
     */
    void monitor(bool silent);
    
    /** Returns number of read errors to detect read overflow errors.
     */
    unsigned char rderror();

    /** Returns number of write errors to detect write overflow errors.
     */
    unsigned char tderror();

    /** Attach a function to call whenever a CAN frame received interrupt is
     *  generated.
     *
     *  @param fptr A pointer to a void function, or 0 to set as none
     */
    void attach(void (*fptr)(void));
   
   /** Attach a member function to call whenever a CAN frame received interrupt
    *  is generated.
    *
    *  @param tptr pointer to the object to call the member function on
    *  @param mptr pointer to the member function to be called
    */
   template<typename T>
   void attach(T* tptr, void (T::*mptr)(void)) {
        if((mptr != NULL) && (tptr != NULL)) {
            _rxirq.attach(tptr, mptr);
            setup_interrupt();
        } else {
            remove_interrupt();
        }
    }
    
private:
    can_t _can;
    FunctionPointer _rxirq;
    
    void setup_interrupt(void);
    void remove_interrupt(void);
};

} // namespace mbed

#endif

#endif    // MBED_CAN_H
