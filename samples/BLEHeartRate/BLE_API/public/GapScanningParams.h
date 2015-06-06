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

#ifndef __GAP_SCANNING_PARAMS_H__
#define __GAP_SCANNING_PARAMS_H__

#include "Gap.h"

class GapScanningParams {
public:
    static const unsigned SCAN_INTERVAL_MIN = 0x0004; /**< Minimum Scan interval in 625 us units, i.e. 2.5 ms. */
    static const unsigned SCAN_INTERVAL_MAX = 0x4000; /**< Maximum Scan interval in 625 us units, i.e. 10.24 s. */
    static const unsigned SCAN_WINDOW_MIN   = 0x0004; /**< Minimum Scan window in 625 us units, i.e. 2.5 ms. */
    static const unsigned SCAN_WINDOW_MAX   = 0x4000; /**< Maximum Scan window in 625 us units, i.e. 10.24 s. */
    static const unsigned SCAN_TIMEOUT_MIN  = 0x0001; /**< Minimum Scan timeout in seconds. */
    static const unsigned SCAN_TIMEOUT_MAX  = 0xFFFF; /**< Maximum Scan timeout in seconds. */

public:
    GapScanningParams(uint16_t interval = SCAN_INTERVAL_MAX,
                      uint16_t window   = SCAN_WINDOW_MAX,
                      uint16_t timeout  = 0,
                      bool     activeScanning = false) : _interval(Gap::MSEC_TO_ADVERTISEMENT_DURATION_UNITS(interval)),
                                                         _window(Gap::MSEC_TO_ADVERTISEMENT_DURATION_UNITS(window)),
                                                         _timeout(timeout),
                                                         _activeScanning(activeScanning) {
        /* stay within limits */
        if (_interval < SCAN_INTERVAL_MIN) {
            _interval = SCAN_INTERVAL_MIN;
        }
        if (_interval > SCAN_INTERVAL_MAX) {
            _interval = SCAN_INTERVAL_MAX;
        }
        if (_window < SCAN_WINDOW_MIN) {
            _window = SCAN_WINDOW_MIN;
        }
        if (_window > SCAN_WINDOW_MAX) {
            _window = SCAN_WINDOW_MAX;
        }
        if (_timeout > SCAN_TIMEOUT_MAX) {
            _timeout = SCAN_TIMEOUT_MAX;
        }
    }

    ble_error_t setInterval(uint16_t newIntervalInMS) {
        uint16_t newInterval = Gap::MSEC_TO_ADVERTISEMENT_DURATION_UNITS(newIntervalInMS);
        if ((newInterval >= SCAN_INTERVAL_MIN) && (newInterval < SCAN_INTERVAL_MAX)) {
            _interval = newInterval;
            return BLE_ERROR_NONE;
        }

        return BLE_ERROR_PARAM_OUT_OF_RANGE;
    }

    ble_error_t setWindow(uint16_t newWindowInMS)     {
        uint16_t newWindow = Gap::MSEC_TO_ADVERTISEMENT_DURATION_UNITS(newWindowInMS);
        if ((newWindow >= SCAN_WINDOW_MIN) && (newWindow < SCAN_WINDOW_MAX)) {
            _window   = newWindow;
            return BLE_ERROR_NONE;
        }

        return BLE_ERROR_PARAM_OUT_OF_RANGE;
    }

    ble_error_t setTimeout(uint16_t newTimeout)   {
        if (newTimeout <= SCAN_TIMEOUT_MAX) {
            _timeout  = newTimeout;
            return BLE_ERROR_NONE;
        }

        return BLE_ERROR_PARAM_OUT_OF_RANGE;
    }

    void setActiveScanning(bool activeScanning) {
        _activeScanning = activeScanning;
    }


    /* @Note: The following return durations in units of 0.625 ms */
    uint16_t getInterval(void) const {return _interval;}
    uint16_t getWindow(void)   const {return _window;  }

    uint16_t getTimeout(void)  const {return _timeout; }
    bool     getActiveScanning(void) const {return _activeScanning;}

private:
    uint16_t _interval; /**< Scan interval in units of 625us (between 2.5ms to 10.24s). */
    uint16_t _window;   /**< Scan window in units of 625us (between 2.5ms to 10.24s). */
    uint16_t _timeout;  /**< Scan timeout between 0x0001 and 0xFFFF in seconds, 0x0000 disables timeout. */
    bool     _activeScanning; /**< obtain not only the advertising data from the peer device, but also their scanResponse if possible. */

private:
    /* disallow copy constructor */
    GapScanningParams(const GapScanningParams &);
    GapScanningParams& operator =(const GapScanningParams &in);
};

#endif // ifndef __GAP_SCANNING_PARAMS_H__