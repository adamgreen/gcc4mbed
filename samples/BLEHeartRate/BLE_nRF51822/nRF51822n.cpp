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

#include "mbed.h"
#include "nRF51822n.h"
#include "nrf_soc.h"

#include "btle/btle.h"
#include "nrf_delay.h"

/**
 * The singleton which represents the nRF51822 transport for the BLEDevice.
 */
static nRF51822n deviceInstance;

/**
 * BLE-API requires an implementation of the following function in order to
 * obtain its transport handle.
 */
BLEDeviceInstanceBase *
createBLEDeviceInstance(void)
{
    return (&deviceInstance);
}

/**************************************************************************/
/*!
    @brief  Constructor
*/
/**************************************************************************/
nRF51822n::nRF51822n(void)
{
}

/**************************************************************************/
/*!
    @brief  Destructor
*/
/**************************************************************************/
nRF51822n::~nRF51822n(void)
{
}

const char *nRF51822n::getVersion(void)
{
    static char versionString[10];
    static bool versionFetched = false;

    if (!versionFetched) {
        ble_version_t version;
        if (sd_ble_version_get(&version) == NRF_SUCCESS) {
            snprintf(versionString, sizeof(versionString), "%u.%u", version.version_number, version.subversion_number);
            versionFetched = true;
        } else {
            strncpy(versionString, "unknown", sizeof(versionString));
        }
    }

    return versionString;
}

/* (Valid values are -40, -20, -16, -12, -8, -4, 0, 4) */
ble_error_t nRF51822n::setTxPower(int8_t txPower)
{
    unsigned rc;
    if ((rc = sd_ble_gap_tx_power_set(txPower)) != NRF_SUCCESS) {
        switch (rc) {
            case NRF_ERROR_BUSY:
                return BLE_STACK_BUSY;
            case NRF_ERROR_INVALID_PARAM:
            default:
                return BLE_ERROR_PARAM_OUT_OF_RANGE;
        }
    }

    return BLE_ERROR_NONE;
}

/**************************************************************************/
/*!
    @brief  Initialises anything required to start using BLE

    @returns    ble_error_t

    @retval     BLE_ERROR_NONE
                Everything executed properly

    @section EXAMPLE

    @code

    @endcode
*/
/**************************************************************************/
ble_error_t nRF51822n::init(void)
{
    /* ToDo: Clear memory contents, reset the SD, etc. */
    btle_init();

    reset();

    return BLE_ERROR_NONE;
}

/**************************************************************************/
/*!
    @brief  Resets the BLE HW, removing any existing services and
            characteristics

    @returns    ble_error_t

    @retval     BLE_ERROR_NONE
                Everything executed properly

    @section EXAMPLE

    @code

    @endcode
*/
/**************************************************************************/
ble_error_t nRF51822n::reset(void)
{
    nrf_delay_us(500000);

    /* Wait for the radio to come back up */
    nrf_delay_us(1000000);

    return BLE_ERROR_NONE;
}

void
nRF51822n::waitForEvent(void)
{
    sd_app_evt_wait();
}
