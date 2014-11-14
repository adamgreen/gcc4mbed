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

#ifndef __NRF51822_GAP_H__
#define __NRF51822_GAP_H__

#include "mbed.h"
#include "blecommon.h"
#include "ble.h"
#include "GapAdvertisingParams.h"
#include "GapAdvertisingData.h"
#include "public/Gap.h"

/**************************************************************************/
/*!
    \brief

*/
/**************************************************************************/
class nRF51Gap : public Gap
{
public:
    static nRF51Gap &getInstance() {
        static nRF51Gap m_instance;
        return m_instance;
    }

    /* Functions that must be implemented from Gap */
    virtual ble_error_t setAddress(addr_type_t  type,  const uint8_t address[ADDR_LEN]);
    virtual ble_error_t getAddress(addr_type_t *typeP, uint8_t addressP[ADDR_LEN]);
    virtual ble_error_t setAdvertisingData(const GapAdvertisingData &, const GapAdvertisingData &);
    virtual ble_error_t startAdvertising(const GapAdvertisingParams &);
    virtual ble_error_t stopAdvertising(void);
    virtual ble_error_t disconnect(DisconnectionReason_t reason);

    virtual ble_error_t setDeviceName(const uint8_t *deviceName);
    virtual ble_error_t getDeviceName(uint8_t *deviceName, unsigned *lengthP);
    virtual ble_error_t setAppearance(uint16_t appearance);
    virtual ble_error_t getAppearance(uint16_t *appearanceP);

    void     setConnectionHandle(uint16_t con_handle);
    uint16_t getConnectionHandle(void);

    virtual ble_error_t getPreferredConnectionParams(ConnectionParams_t *params);
    virtual ble_error_t setPreferredConnectionParams(const ConnectionParams_t *params);
    virtual ble_error_t updateConnectionParams(Handle_t handle, const ConnectionParams_t *params);

private:
    uint16_t m_connectionHandle;
    nRF51Gap() {
        m_connectionHandle = BLE_CONN_HANDLE_INVALID;
    }

    nRF51Gap(nRF51Gap const &);
    void operator=(nRF51Gap const &);
};

#endif // ifndef __NRF51822_GAP_H__
