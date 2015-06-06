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

#include "Gap.h"
#include "GattService.h"
#include "GattAttribute.h"
#include "GattServerEvents.h"
#include "GattCharacteristicCallbackParams.h"
#include "CallChainOfFunctionPointersWithContext.h"

class GattServer {
public:
    /* Event callback handlers. */
    typedef void (*EventCallback_t)(GattAttribute::Handle_t attributeHandle);
    typedef void (*ServerEventCallback_t)(void);                    /**< independent of any particular attribute */

protected:
    GattServer() :
        serviceCount(0),
        characteristicCount(0),
        onDataSent(),
        onDataWritten(),
        onDataRead(),
        onUpdatesEnabled(NULL),
        onUpdatesDisabled(NULL),
        onConfirmationReceived(NULL) {
        /* empty */
    }

    friend class BLEDevice;
private:
    /* These functions must be defined in the sub-class */
    virtual ble_error_t addService(GattService &)                                                               = 0;
    virtual ble_error_t readValue(GattAttribute::Handle_t attributeHandle, uint8_t buffer[], uint16_t *lengthP) = 0;
    virtual ble_error_t readValue(Gap::Handle_t connectionHandle, GattAttribute::Handle_t attributeHandle, uint8_t buffer[], uint16_t *lengthP) = 0;
    virtual ble_error_t updateValue(GattAttribute::Handle_t, const uint8_t[], uint16_t, bool localOnly = false) = 0;
    virtual ble_error_t updateValue(Gap::Handle_t connectionHandle, GattAttribute::Handle_t, const uint8_t[], uint16_t, bool localOnly = false) = 0;
    virtual ble_error_t initializeGATTDatabase(void)                                                            = 0;

    // ToDo: For updateValue, check the CCCD to see if the value we are
    // updating has the notify or indicate bits sent, and if BOTH are set
    // be sure to call sd_ble_gatts_hvx() twice with notify then indicate!
    // Strange use case, but valid and must be covered!

    void setOnDataSent(void (*callback)(unsigned count)) {onDataSent.add(callback);}
    template <typename T>
    void setOnDataSent(T *objPtr, void (T::*memberPtr)(unsigned count)) {
        onDataSent.add(objPtr, memberPtr);
    }
    void setOnDataWritten(void (*callback)(const GattCharacteristicWriteCBParams *eventDataP)) {onDataWritten.add(callback);}
    template <typename T>
    void setOnDataWritten(T *objPtr, void (T::*memberPtr)(const GattCharacteristicWriteCBParams *context)) {
        onDataWritten.add(objPtr, memberPtr);
    }

    /**
     * A virtual function to allow underlying stacks to indicate if they support
     * onDataRead(). It should be overridden to return true as applicable.
     */
    virtual bool isOnDataReadAvailable() const {
        return false;
    }
    ble_error_t setOnDataRead(void (*callback)(const GattCharacteristicReadCBParams *eventDataP)) {
        if (!isOnDataReadAvailable()) {
            return BLE_ERROR_NOT_IMPLEMENTED;
        }

        onDataRead.add(callback);
        return BLE_ERROR_NONE;
    }
    template <typename T>
    ble_error_t setOnDataRead(T *objPtr, void (T::*memberPtr)(const GattCharacteristicReadCBParams *context)) {
        if (!isOnDataReadAvailable()) {
            return BLE_ERROR_NOT_IMPLEMENTED;
        }

        onDataRead.add(objPtr, memberPtr);
        return BLE_ERROR_NONE;
    }
    void setOnUpdatesEnabled(EventCallback_t callback)       {onUpdatesEnabled       = callback;}
    void setOnUpdatesDisabled(EventCallback_t callback)      {onUpdatesDisabled      = callback;}
    void setOnConfirmationReceived(EventCallback_t callback) {onConfirmationReceived = callback;}

protected:
    void handleDataWrittenEvent(const GattCharacteristicWriteCBParams *params) {
        if (onDataWritten.hasCallbacksAttached()) {
            onDataWritten.call(params);
        }
    }

    void handleDataReadEvent(const GattCharacteristicReadCBParams *params) {
        if (onDataRead.hasCallbacksAttached()) {
            onDataRead.call(params);
        }
    }

    void handleEvent(GattServerEvents::gattEvent_e type, GattAttribute::Handle_t charHandle) {
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
            default:
                break;
        }
    }

    void handleDataSentEvent(unsigned count) {
        if (onDataSent.hasCallbacksAttached()) {
            onDataSent.call(count);
        }
    }

protected:
    uint8_t serviceCount;
    uint8_t characteristicCount;

private:
    CallChainOfFunctionPointersWithContext<unsigned>                                onDataSent;
    CallChainOfFunctionPointersWithContext<const GattCharacteristicWriteCBParams *> onDataWritten;
    CallChainOfFunctionPointersWithContext<const GattCharacteristicReadCBParams *>  onDataRead;
    EventCallback_t                                                                 onUpdatesEnabled;
    EventCallback_t                                                                 onUpdatesDisabled;
    EventCallback_t                                                                 onConfirmationReceived;

private:
    /* disallow copy and assignment */
    GattServer(const GattServer &);
    GattServer& operator=(const GattServer &);
};

#endif // ifndef __GATT_SERVER_H__