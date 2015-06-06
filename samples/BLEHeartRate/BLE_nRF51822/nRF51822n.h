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

#ifndef __NRF51822_H__
#define __NRF51822_H__

#include "mbed.h"
#include "blecommon.h"
#include "BLEDevice.h"
#include "nRF51Gap.h"
#include "nRF51GattServer.h"
#include "btle.h"
#include "btle_security.h"

class nRF51822n : public BLEDeviceInstanceBase
{
public:
    nRF51822n(void);
    virtual ~nRF51822n(void);

    virtual const char *getVersion(void);

    virtual Gap        &getGap()        {
        return nRF51Gap::getInstance();
    };
    virtual GattServer &getGattServer() {
        return nRF51GattServer::getInstance();
    };

    virtual ble_error_t setTxPower(int8_t txPower);
    virtual void        getPermittedTxPowerValues(const int8_t **valueArrayPP, size_t *countP);

    virtual ble_error_t init(void);
    virtual ble_error_t shutdown(void);
    virtual ble_error_t reset(void);
    virtual ble_error_t initializeSecurity(bool                          enableBonding = true,
                                           bool                          requireMITM   = true,
                                           Gap::SecurityIOCapabilities_t iocaps        = Gap::IO_CAPS_NONE,
                                           const Gap::Passkey_t          passkey       = NULL) {
        return btle_initializeSecurity(enableBonding, requireMITM, iocaps, passkey);
    }
    virtual void        waitForEvent(void);
};

#endif