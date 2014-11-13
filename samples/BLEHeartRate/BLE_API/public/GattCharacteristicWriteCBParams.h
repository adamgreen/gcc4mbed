/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __GATT_CHARACTERISTIC_WRITE_CB_PARAMS_H__
#define __GATT_CHARACTERISTIC_WRITE_CB_PARAMS_H__

struct GattCharacteristicWriteCBParams {
    GattAttribute::Handle_t charHandle;
    enum Type {
        GATTS_CHAR_OP_INVALID               = 0x00,  /**< Invalid Operation. */
        GATTS_CHAR_OP_WRITE_REQ             = 0x01,  /**< Write Request. */
        GATTS_CHAR_OP_WRITE_CMD             = 0x02,  /**< Write Command. */
        GATTS_CHAR_OP_SIGN_WRITE_CMD        = 0x03,  /**< Signed Write Command. */
        GATTS_CHAR_OP_PREP_WRITE_REQ        = 0x04,  /**< Prepare Write Request. */
        GATTS_CHAR_OP_EXEC_WRITE_REQ_CANCEL = 0x05,  /**< Execute Write Request: Cancel all prepared writes. */
        GATTS_CHAR_OP_EXEC_WRITE_REQ_NOW    = 0x06,  /**< Execute Write Request: Immediately execute all prepared writes. */
    } op;                  /**< Type of write operation, */
    uint16_t       offset; /**< Offset for the write operation. */
    uint16_t       len;    /**< Length of the incoming data. */
    const uint8_t *data;   /**< Incoming data, variable length. */
};

#endif /*__GATT_CHARACTERISTIC_WRITE_CB_PARAMS_H__*/
