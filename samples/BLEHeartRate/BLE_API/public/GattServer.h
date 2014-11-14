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

#ifndef __GATT_SERVER_H__
#define __GATT_SERVER_H__

#include "mbed.h"
#include "blecommon.h"
#include "GattService.h"
#include "GattServerEvents.h"
#include "GattCharacteristicWriteCBParams.h"
#include "CallChainOfFunctionPointersWithContext.h"

/**************************************************************************/
/*!
    \brief
    The base class used to abstract GATT Server functionality to a specific
    radio transceiver, SOC or BLE Stack.
*/
/**************************************************************************/
class GattServer
{
public:
    /* These functions must be defined in the sub-class */
    virtual ble_error_t addService(GattService &) = 0;
    virtual ble_error_t readValue(uint16_t handle, uint8_t buffer[], uint16_t *const lengthP) = 0;
    virtual ble_error_t updateValue(uint16_t, uint8_t[], uint16_t, bool localOnly = false) = 0;

    // ToDo: For updateValue, check the CCCD to see if the value we are
    // updating has the notify or indicate bits sent, and if BOTH are set
    // be sure to call sd_ble_gatts_hvx() twice with notify then indicate!
    // Strange use case, but valid and must be covered!

    /* Event callback handlers. */
    typedef void (*EventCallback_t)(uint16_t attributeHandle);
    typedef void (*ServerEventCallback_t)(void);                    /**< independent of any particular attribute */
    typedef void (*ServerEventCallbackWithCount_t)(unsigned count); /**< independent of any particular attribute */
    void setOnDataSent(ServerEventCallbackWithCount_t callback) {
        onDataSent = callback;
    }
    void setOnDataWritten(void (*callback)(const GattCharacteristicWriteCBParams *eventDataP)) {
        onDataWritten.add(callback);
    }
    template <typename T>
    void setOnDataWritten(T *objPtr, void (T::*memberPtr)(const GattCharacteristicWriteCBParams *context)) {
        onDataWritten.add(objPtr, memberPtr);
    }
    void setOnUpdatesEnabled(EventCallback_t callback) {
        onUpdatesEnabled = callback;
    }
    void setOnUpdatesDisabled(EventCallback_t callback) {
        onUpdatesDisabled = callback;
    }
    void setOnConfirmationReceived(EventCallback_t callback) {
        onConfirmationReceived = callback;
    }

    void handleDataWrittenEvent(const GattCharacteristicWriteCBParams *params) {
        if (onDataWritten.hasCallbacksAttached()) {
            onDataWritten.call(params);
        }
    }

    void handleEvent(GattServerEvents::gattEvent_e type, uint16_t charHandle) {
        switch (type) {
            case GattServerEvents::GATT_EVENT_UPDATES_ENABLED:
                if (onUpdatesEnabled) {
                    onUpdatesEnabled(charHandle);
                }
                break;
            case GattServerEvents::GATT_EVENT_UPDATES_DISABLED:
                if (onUpdatesDisabled) {
                    onUpdatesDisabled(charHandle);
                }
                break;
            case GattServerEvents::GATT_EVENT_CONFIRMATION_RECEIVED:
                if (onConfirmationReceived) {
                    onConfirmationReceived(charHandle);
                }
                break;
        }
    }

    void handleDataSentEvent(unsigned count) {
        if (onDataSent) {
            onDataSent(count);
        }
    }

protected:
    GattServer() : serviceCount(0), characteristicCount(0), onDataSent(NULL), onDataWritten(), onUpdatesEnabled(NULL), onUpdatesDisabled(NULL), onConfirmationReceived(NULL) {
        /* empty */
    }

protected:
    uint8_t serviceCount;
    uint8_t characteristicCount;
    uint8_t descriptorCount;

private:
    ServerEventCallbackWithCount_t onDataSent;
    CallChainOfFunctionPointersWithContext<const GattCharacteristicWriteCBParams *> onDataWritten;
    EventCallback_t                onUpdatesEnabled;
    EventCallback_t                onUpdatesDisabled;
    EventCallback_t                onConfirmationReceived;
};

#endif // ifndef __GATT_SERVER_H__
