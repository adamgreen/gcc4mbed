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

#ifndef __BLE_DEVICE__
#define __BLE_DEVICE__

#include "blecommon.h"
#include "Gap.h"
#include "GattServer.h"
#include "GapScanningParams.h"
#include "BLEDeviceInstanceBase.h"

/**
 * The base class used to abstract away BLE capable radio transceivers or SOCs,
 * to enable this BLE API to work with any radio transparently.
 */
class BLEDevice
{
public:
    /**
     * Initialize the BLE controller. This should be called before using
     * anything else in the BLE_API.
     */
    ble_error_t init();
    ble_error_t reset(void);

    /**
     * Purge the BLE stack of GATT and GAP state. init() must be called afterwards to re-instate services and GAP state.
     */
    ble_error_t shutdown(void);

    /* GAP specific APIs */
public:
    /**
     * Set the BTLE MAC address and type.
     * @return BLE_ERROR_NONE on success.
     */
    ble_error_t setAddress(Gap::addr_type_t type, const Gap::address_t address);

    /**
     * Fetch the BTLE MAC address and type.
     * @return BLE_ERROR_NONE on success.
     */
    ble_error_t getAddress(Gap::addr_type_t *typeP, Gap::address_t address);

    /**
     * @param[in] advType
     *              The GAP advertising mode to use for this device. Valid
     *              values are defined in AdvertisingType:
     *
     *              \par ADV_NON_CONNECTABLE_UNDIRECTED
     *              All connections to the peripheral device will be refused.
     *
     *              \par ADV_CONNECTABLE_DIRECTED
     *              Only connections from a pre-defined central device will be
     *              accepted.
     *
     *              \par ADV_CONNECTABLE_UNDIRECTED
     *              Any central device can connect to this peripheral.
     *
     *              \par ADV_SCANNABLE_UNDIRECTED
     *              Include support for Scan Response payloads.
     *
     *              \par
     *              See Bluetooth Core Specification 4.0 (Vol. 3), Part C,
     *              Section 9.3 and Core Specification 4.0 (Vol. 6), Part B,
     *              Section 2.3.1 for further information on GAP connection
     *              modes
     */
    void        setAdvertisingType(GapAdvertisingParams::AdvertisingType);

    /**
     * @param[in] interval
     *              Advertising interval in units of milliseconds. Advertising
     *              is disabled if interval is 0. If interval is smaller than
     *              the minimum supported value, then the minimum supported
     *              value is used instead.
     *
     *              \par
     *              Decreasing this value will allow central devices to detect
     *              your peripheral faster at the expense of more power being
     *              used by the radio due to the higher data transmit rate.
     *
     *              \par
     *              This field must be set to 0 if connectionMode is equal
     *              to ADV_CONNECTABLE_DIRECTED
     *
     *              \par
     *              See Bluetooth Core Specification, Vol 3., Part C,
     *              Appendix A for suggested advertising intervals.
     *
     * @Note: [WARNING] This API previously used 0.625ms as the unit for its
     * 'interval' argument. That required an explicit conversion from
     * milliseconds using Gap::MSEC_TO_GAP_DURATION_UNITS(). This conversion is
     * no longer required as the new units are milliseconds. Any application
     * code depending on the old semantics would need to be updated accordingly.
     */
    void        setAdvertisingInterval(uint16_t interval);

    /**
     * @return Minimum Advertising interval in milliseconds.
     */
    uint16_t    getMinAdvertisingInterval(void) const;
    /**
     * @return Minimum Advertising interval in milliseconds for non connectible mode.
     */
    uint16_t    getMinNonConnectableAdvertisingInterval(void) const;
    /**
     * @return Maximum Advertising interval in milliseconds.
     */
    uint16_t    getMaxAdvertisingInterval(void) const;

    /**
     * @param[in] timeout
     *              Advertising timeout between 0x1 and 0x3FFF (1 and 16383)
     *              in seconds.  Enter 0 to disable the advertising timeout.
     */
    void        setAdvertisingTimeout(uint16_t timeout);

    /**
     * Please refer to the APIs above.
     */
    void        setAdvertisingParams(const GapAdvertisingParams &advParams);

    /**
     * This API is typically used as an internal helper to udpate the transport
     * backend with advertising data before starting to advertise. It may also
     * be explicity used to dynamically reset the accumulated advertising
     * payload and scanResponse; to do this, the application can clear and re-
     * accumulate a new advertising payload (and scanResponse) before using this
     * API.
     */
    ble_error_t setAdvertisingPayload(void);

    /**
     * Reset any advertising payload prepared from prior calls to
     * accumulateAdvertisingPayload().
     *
     * Note: This should be followed by a call to setAdvertisingPayload() or
     * startAdvertising() before the update takes effect.
     */
    void        clearAdvertisingPayload(void);

    /**
     * Accumulate an AD structure in the advertising payload. Please note that
     * the payload is limited to 31 bytes. The SCAN_RESPONSE message may be used
     * as an additional 31 bytes if the advertising payload proves to be too
     * small.
     *
     * @param  flags
     *         The flags to be added. Multiple flags may be specified in
     *         combination.
     */
    ble_error_t accumulateAdvertisingPayload(uint8_t flags);

    /**
     * Accumulate an AD structure in the advertising payload. Please note that
     * the payload is limited to 31 bytes. The SCAN_RESPONSE message may be used
     * as an additional 31 bytes if the advertising payload proves to be too
     * small.
     *
     * @param  app
     *         The appearance of the peripheral.
     */
    ble_error_t accumulateAdvertisingPayload(GapAdvertisingData::Appearance app);

    /**
     * Accumulate an AD structure in the advertising payload. Please note that
     * the payload is limited to 31 bytes. The SCAN_RESPONSE message may be used
     * as an additional 31 bytes if the advertising payload proves to be too
     * small.
     *
     * @param  app
     *         The max transmit power to be used by the controller. This is
     *         only a hint.
     */
    ble_error_t accumulateAdvertisingPayloadTxPower(int8_t power);

    /**
     * Accumulate a variable length byte-stream as an AD structure in the
     * advertising payload. Please note that the payload is limited to 31 bytes.
     * The SCAN_RESPONSE message may be used as an additional 31 bytes if the
     * advertising payload proves to be too small.
     *
     * @param  type The type which describes the variable length data.
     * @param  data data bytes.
     * @param  len  length of data.
     */
    ble_error_t accumulateAdvertisingPayload(GapAdvertisingData::DataType type, const uint8_t *data, uint8_t len);

    /**
     * Accumulate a variable length byte-stream as an AD structure in the
     * scanResponse payload.
     *
     * @param  type The type which describes the variable length data.
     * @param  data data bytes.
     * @param  len  length of data.
     */
    ble_error_t accumulateScanResponse(GapAdvertisingData::DataType type, const uint8_t *data, uint8_t len);

    /**
     * Reset any scan response prepared from prior calls to
     * accumulateScanResponse().
     *
     * Note: This should be followed by a call to setAdvertisingPayload() or
     * startAdvertising() before the update takes effect.
     */
    void        clearScanResponse(void);

    /**
     * Start advertising (GAP Discoverable, Connectable modes, Broadcast
     * Procedure).
     */
    ble_error_t startAdvertising(void);

    /**
     * Stop advertising (GAP Discoverable, Connectable modes, Broadcast
     * Procedure).
     */
    ble_error_t stopAdvertising(void);

    /**
     * Setup parameters for GAP scanning--i.e. observer mode.
     * @param  interval Scan interval (in milliseconds) [valid values lie between 2.5ms and 10.24s].
     * @param  window   Scan Window (in milliseconds) [valid values lie between 2.5ms and 10.24s].
     * @param  timeout  Scan timeout (in seconds) between 0x0001 and 0xFFFF, 0x0000 disables timeout.
     * @param  activeScanning Set to True if active-scanning is required. This is used to fetch the
     *                        scan response from a peer if possible.
     *
     * The scanning window divided by the interval determines the duty cycle for
     * scanning. For example, if the interval is 100ms and the window is 10ms,
     * then the controller will scan for 10 percent of the time. It is possible
     * to have the interval and window set to the same value. In this case,
     * scanning is continuous, with a change of scanning frequency once every
     * interval.
     *
     * Once the scanning parameters have been configured, scanning can be
     * enabled by using startScan().
     *
     * @Note: The scan interval and window are recommendations to the BLE stack.
     */
    ble_error_t setScanParams(uint16_t interval       = GapScanningParams::SCAN_INTERVAL_MAX,
                              uint16_t window         = GapScanningParams::SCAN_WINDOW_MAX,
                              uint16_t timeout        = 0,
                              bool     activeScanning = false);
    ble_error_t setScanInterval(uint16_t interval);
    ble_error_t setScanWindow  (uint16_t window);
    ble_error_t setScanTimeout (uint16_t timeout);
    void        setActiveScan  (bool     activeScanning);

    /**
     * Start scanning (Observer Procedure) based on the scan-params currently
     * in effect.
     *
     * @param  callback The application callback to be invoked upon receiving
     *     every advertisement report. Can be passed in as NULL, in which case
     *     scanning may not be enabled at all.
     */
    ble_error_t startScan(void (*callback)(const Gap::AdvertisementCallbackParams_t *params));

    /**
     * Start scanning (Observer Procedure) based on the scan-params currently
     * in effect.
     *
     * @param[in] object
     * @param[in] callbackMember
     *                The above pair of parameters define the callback object
     *                and member function to receive the advertisement params.
     */
    template<typename T>
    ble_error_t startScan(T *object, void (T::*memberCallback)(const Gap::AdvertisementCallbackParams_t *params));

    /**
     * Stop scanning. The current scanning parameters remain in effect.
     *
     * @retval BLE_ERROR_NONE if successfully stopped scanning procedure.
     */
    ble_error_t stopScan(void);

    /**
     * This call initiates the disconnection procedure, and its completion will
     * be communicated to the application with an invocation of the
     * onDisconnection callback.
     *
     * @param  reason
     *           The reason for disconnection to be sent back to the peer.
     */
    ble_error_t disconnect(Gap::DisconnectionReason_t reason);

    /* APIs to set GAP callbacks. */
    void onTimeout(Gap::EventCallback_t timeoutCallback);

    void onConnection(Gap::ConnectionEventCallback_t connectionCallback);
    /**
     * Used to setup a callback for GAP disconnection.
     */
    void onDisconnection(Gap::DisconnectionEventCallback_t disconnectionCallback);

    /**
     * Append to a chain of callbacks to be invoked upon disconnection; these
     * callbacks receive no context and are therefore different from the
     * onDisconnection callback.
     */
    template<typename T>
    void addToDisconnectionCallChain(T *tptr, void (T::*mptr)(void));

    /**
     * Add a callback for the GATT event DATA_SENT (which is triggered when
     * updates are sent out by GATT in the form of notifications).
     *
     * @Note: it is possible to chain together multiple onDataSent callbacks
     * (potentially from different modules of an application) to receive updates
     * to characteristics.
     *
     * @Note: it is also possible to setup a callback into a member function of
     * some object.
     */
    void onDataSent(void (*callback)(unsigned count));
    template <typename T> void onDataSent(T * objPtr, void (T::*memberPtr)(unsigned count));

    /**
     * Setup a callback for when a characteristic has its value updated by a
     * client.
     *
     * @Note: it is possible to chain together multiple onDataWritten callbacks
     * (potentially from different modules of an application) to receive updates
     * to characteristics. Many services, such as DFU and UART add their own
     * onDataWritten callbacks behind the scenes to trap interesting events.
     *
     * @Note: it is also possible to setup a callback into a member function of
     * some object.
     */
    void onDataWritten(void (*callback)(const GattCharacteristicWriteCBParams *eventDataP));
    template <typename T> void onDataWritten(T * objPtr, void (T::*memberPtr)(const GattCharacteristicWriteCBParams *context));

    /**
     * Setup a callback for when a characteristic is being read by a client.
     *
     * @Note: this functionality may not be available on all underlying stacks.
     * You could use GattCharacteristic::setReadAuthorizationCallback() as an
     * alternative.
     *
     * @Note: it is possible to chain together multiple onDataRead callbacks
     * (potentially from different modules of an application) to receive updates
     * to characteristics. Services may add their own onDataRead callbacks
     * behind the scenes to trap interesting events.
     *
     * @Note: it is also possible to setup a callback into a member function of
     * some object.
     *
     * @return BLE_ERROR_NOT_IMPLEMENTED if this functionality isn't available;
     *         else BLE_ERROR_NONE.
     */
    ble_error_t onDataRead(void (*callback)(const GattCharacteristicReadCBParams *eventDataP));
    template <typename T> ble_error_t onDataRead(T * objPtr, void (T::*memberPtr)(const GattCharacteristicReadCBParams *context));

    void onUpdatesEnabled(GattServer::EventCallback_t callback);
    void onUpdatesDisabled(GattServer::EventCallback_t callback);
    void onConfirmationReceived(GattServer::EventCallback_t callback);

    /**
     * Radio Notification is a feature that enables ACTIVE and INACTIVE
     * (nACTIVE) signals from the stack that notify the application when the
     * radio is in use. The signal is sent using software interrupt.
     *
     * The ACTIVE signal is sent before the Radio Event starts. The nACTIVE
     * signal is sent at the end of the Radio Event. These signals can be used
     * by the application programmer to synchronize application logic with radio
     * activity. For example, the ACTIVE signal can be used to shut off external
     * devices to manage peak current drawn during periods when the radio is on,
     * or to trigger sensor data collection for transmission in the Radio Event.
     *
     * @param callback
     *          The application handler to be invoked in response to a radio
     *          ACTIVE/INACTIVE event.
     */
    void onRadioNotification(Gap::RadioNotificationEventCallback_t callback);

    /**
     * Add a service declaration to the local server ATT table. Also add the
     * characteristics contained within.
     */
    ble_error_t addService(GattService &service);

    /**
     * Returns the current GAP state of the device using a bitmask which
     * describes whether the device is advertising and/or connected.
     */
    Gap::GapState_t getGapState(void) const;

    /**
     * @param[in/out]  lengthP
     *     input:  Length in bytes to be read,
     *     output: Total length of attribute value upon successful return.
     */
    ble_error_t readCharacteristicValue(GattAttribute::Handle_t attributeHandle, uint8_t *buffer, uint16_t *lengthP);
    /**
     * A version of the same as above with connection handle parameter to allow fetches for connection-specific multivalued attribtues (such as the CCCDs).
     */
    ble_error_t readCharacteristicValue(Gap::Handle_t connectionHandle, GattAttribute::Handle_t attributeHandle, uint8_t *buffer, uint16_t *lengthP);

    /**
     * @param  localOnly
     *         Only update the characteristic locally regardless of notify/indicate flags in the CCCD.
     */
    ble_error_t updateCharacteristicValue(GattAttribute::Handle_t attributeHandle, const uint8_t *value, uint16_t size, bool localOnly = false);
    /**
     * A version of the same as above with connection handle parameter to allow updates for connection-specific multivalued attribtues (such as the CCCDs).
     */
    ble_error_t updateCharacteristicValue(Gap::Handle_t connectionHandle, GattAttribute::Handle_t attributeHandle, const uint8_t *value, uint16_t size, bool localOnly = false);

    /**
     * Yield control to the BLE stack or to other tasks waiting for events. This
     * is a sleep function which will return when there is an application
     * specific interrupt, but the MCU might wake up several times before
     * returning (to service the stack). This is not always interchangeable with
     * WFE().
     */
    void waitForEvent(void);

    ble_error_t getPreferredConnectionParams(Gap::ConnectionParams_t *params);
    ble_error_t setPreferredConnectionParams(const Gap::ConnectionParams_t *params);
    ble_error_t updateConnectionParams(Gap::Handle_t handle, const Gap::ConnectionParams_t *params);

    /**
     * This call allows the application to get the BLE stack version information.
     *
     * @return  A pointer to a const string representing the version.
     *          Note: The string is owned by the BLE_API.
     */
    const char *getVersion(void);

    /**
     * Set the device name characteristic in the GAP service.
     * @param  deviceName The new value for the device-name. This is a UTF-8 encoded, <b>NULL-terminated</b> string.
     */
    ble_error_t setDeviceName(const uint8_t *deviceName);

    /**
     * Get the value of the device name characteristic in the GAP service.
     * @param[out]    deviceName Pointer to an empty buffer where the UTF-8 *non NULL-
     *                           terminated* string will be placed. Set this
     *                           value to NULL in order to obtain the deviceName-length
     *                           from the 'length' parameter.
     *
     * @param[in/out] lengthP    (on input) Length of the buffer pointed to by deviceName;
     *                           (on output) the complete device name length (without the
     *                           null terminator).
     *
     * @note          If the device name is longer than the size of the supplied buffer,
     *                length will return the complete device name length,
     *                and not the number of bytes actually returned in deviceName.
     *                The application may use this information to retry with a suitable buffer size.
     *
     * Sample use:
     *     uint8_t deviceName[20];
     *     unsigned length = sizeof(deviceName);
     *     ble.getDeviceName(deviceName, &length);
     *     if (length < sizeof(deviceName)) {
     *         deviceName[length] = 0;
     *     }
     *     DEBUG("length: %u, deviceName: %s\r\n", length, deviceName);
     */
    ble_error_t getDeviceName(uint8_t *deviceName, unsigned *lengthP);

    /**
     * Set the appearance characteristic in the GAP service.
     * @param[in]  appearance The new value for the device-appearance.
     */
    ble_error_t setAppearance(uint16_t appearance);

    /**
     * Set the appearance characteristic in the GAP service.
     * @param[out]  appearance The new value for the device-appearance.
     */
    ble_error_t getAppearance(uint16_t *appearanceP);

    /**
     * Set the radio's transmit power.
     * @param[in] txPower Radio transmit power in dBm.
     */
    ble_error_t setTxPower(int8_t txPower);

    /**
     * Query the underlying stack for permitted arguments for setTxPower().
     *
     * @param[out] valueArrayPP
     *                 Out parameter to receive the immutable array of Tx values.
     * @param[out] countP
     *                 Out parameter to receive the array's size.
     */
    void getPermittedTxPowerValues(const int8_t **valueArrayPP, size_t *countP);

    /**
     * Enable the BLE stack's Security Manager. The Security Manager implements
     * the actual cryptographic algorithms and protocol exchanges that allow two
     * devices to securely exchange data and privately detect each other.
     * Calling this API is a prerequisite for encryption and pairing (bonding).
     *
     * @param[in]  enableBonding Allow for bonding.
     * @param[in]  requireMITM   Require protection for man-in-the-middle attacks.
     * @param[in]  iocaps        To specify IO capabilities of this peripheral,
     *                           such as availability of a display or keyboard to
     *                           support out-of-band exchanges of security data.
     * @param[in]  passkey       To specify a static passkey.
     *
     * @return BLE_ERROR_NONE on success.
     */
    ble_error_t initializeSecurity(bool                          enableBonding = true,
                                   bool                          requireMITM   = true,
                                   Gap::SecurityIOCapabilities_t iocaps        = Gap::IO_CAPS_NONE,
                                   const Gap::Passkey_t          passkey       = NULL);

    /**
     * Setup a callback for when the security setup procedure (key generation
     * and exchange) for a link has started. This will be skipped for bonded
     * devices. The callback is passed in parameters received from the peer's
     * security request: bool allowBonding, bool requireMITM, and
     * SecurityIOCapabilities_t.
     */
    void onSecuritySetupInitiated(Gap::SecuritySetupInitiatedCallback_t callback);

    /**
     * Setup a callback for when the security setup procedure (key generation
     * and exchange) for a link has completed. This will be skipped for bonded
     * devices. The callback is passed in the success/failure status of the
     * security setup procedure.
     */
    void onSecuritySetupCompleted(Gap::SecuritySetupCompletedCallback_t callback);

    /**
     * Setup a callback for when a link with the peer is secured. For bonded
     * devices, subsequent reconnections with bonded peer will result only in
     * this callback when the link is secured and setup procedures will not
     * occur unless the bonding information is either lost or deleted on either
     * or both sides. The callback is passed in a Gap::SecurityMode_t according
     * to the level of security in effect for the secured link.
     */
    void onLinkSecured(Gap::LinkSecuredCallback_t callback);

    /**
     * Setup a callback for successful bonding; i.e. that link-specific security
     * context is stored persistently for a peer device.
     */
    void onSecurityContextStored(Gap::HandleSpecificEvent_t callback);

    /**
     * Setup a callback for when the passkey needs to be displayed on a
     * peripheral with DISPLAY capability. This happens when security is
     * configured to prevent Man-In-The-Middle attacks, and a PIN (or passkey)
     * needs to be exchanged between the peers to authenticate the connection
     * attempt.
     */
    void onPasskeyDisplay(Gap::PasskeyDisplayCallback_t callback);

    /**
     * Get the security status of a connection.
     *
     * @param[in]  connectionHandle   Handle to identify the connection.
     * @param[out] securityStatusP    security status.
     *
     * @return BLE_SUCCESS Or appropriate error code indicating reason for failure.
     */
    ble_error_t getLinkSecurity(Gap::Handle_t connectionHandle, Gap::LinkSecurityStatus_t *securityStatusP);

    /**
     * Delete all peer device context and all related bonding information from
     * the database within the security manager.
     *
     * @retval BLE_ERROR_NONE             On success, else an error code indicating reason for failure.
     * @retval BLE_ERROR_INVALID_STATE    If the API is called without module initialization and/or
     *                                    application registration.
     */
    ble_error_t purgeAllBondingState(void);

public:
    BLEDevice() : transport(createBLEDeviceInstance()), advParams(), advPayload(), scanResponse(), needToSetAdvPayload(true), scanningParams() {
        advPayload.clear();
        scanResponse.clear();
    }

private:
    BLEDeviceInstanceBase *const transport; /* the device specific backend */

    GapAdvertisingParams advParams;
    GapAdvertisingData   advPayload;
    GapAdvertisingData   scanResponse;

    /* Accumulation of AD structures in the advertisement payload should
     * eventually result in a call to the target's setAdvertisingData() before
     * the server begins advertising. This flag marks the status of the pending update.*/
    bool                 needToSetAdvPayload;

    GapScanningParams    scanningParams;
};

/* BLEDevice methods. Most of these simply forward the calls to the underlying
 * transport.*/

inline ble_error_t
BLEDevice::reset(void)
{
    return transport->reset();
}

inline ble_error_t
BLEDevice::shutdown(void)
{
    clearAdvertisingPayload();
    return transport->shutdown();
}

inline ble_error_t
BLEDevice::setAddress(Gap::addr_type_t type, const Gap::address_t address)
{
    return transport->getGap().setAddress(type, address);
}

inline ble_error_t
BLEDevice::getAddress(Gap::addr_type_t *typeP, Gap::address_t address)
{
    return transport->getGap().getAddress(typeP, address);
}

inline void
BLEDevice::setAdvertisingType(GapAdvertisingParams::AdvertisingType advType)
{
    advParams.setAdvertisingType(advType);
}

inline void
BLEDevice::setAdvertisingInterval(uint16_t interval)
{
    if (interval == 0) {
        stopAdvertising();
    } else if (interval < getMinAdvertisingInterval()) {
        interval = getMinAdvertisingInterval();
    }
    advParams.setInterval(Gap::MSEC_TO_ADVERTISEMENT_DURATION_UNITS(interval));
}

inline uint16_t
BLEDevice::getMinAdvertisingInterval(void) const {
    return transport->getGap().getMinAdvertisingInterval();
}

inline uint16_t
BLEDevice::getMinNonConnectableAdvertisingInterval(void) const {
    return transport->getGap().getMinNonConnectableAdvertisingInterval();
}

inline uint16_t
BLEDevice::getMaxAdvertisingInterval(void) const {
    return transport->getGap().getMaxAdvertisingInterval();
}

inline void
BLEDevice::setAdvertisingTimeout(uint16_t timeout)
{
    advParams.setTimeout(timeout);
}

inline void
BLEDevice::setAdvertisingParams(const GapAdvertisingParams &newAdvParams)
{
    advParams = newAdvParams;
}

inline void
BLEDevice::clearAdvertisingPayload(void)
{
    needToSetAdvPayload = true;
    advPayload.clear();
}

inline ble_error_t
BLEDevice::accumulateAdvertisingPayload(uint8_t flags)
{
    needToSetAdvPayload = true;
    return advPayload.addFlags(flags);
}

inline ble_error_t
BLEDevice::accumulateAdvertisingPayload(GapAdvertisingData::Appearance app)
{
    needToSetAdvPayload = true;
    transport->getGap().setAppearance(app);
    return advPayload.addAppearance(app);
}

inline ble_error_t
BLEDevice::accumulateAdvertisingPayloadTxPower(int8_t txPower)
{
    needToSetAdvPayload = true;
    return advPayload.addTxPower(txPower);
}

inline ble_error_t
BLEDevice::accumulateAdvertisingPayload(GapAdvertisingData::DataType type, const uint8_t *data, uint8_t len)
{
    needToSetAdvPayload = true;
    if (type == GapAdvertisingData::COMPLETE_LOCAL_NAME) {
        transport->getGap().setDeviceName(data);
    }
    return advPayload.addData(type, data, len);
}

inline ble_error_t
BLEDevice::accumulateScanResponse(GapAdvertisingData::DataType type, const uint8_t *data, uint8_t len)
{
    needToSetAdvPayload = true;
    return scanResponse.addData(type, data, len);
}

inline void
BLEDevice::clearScanResponse(void)
{
    needToSetAdvPayload = true;
    scanResponse.clear();
}

inline ble_error_t
BLEDevice::setAdvertisingPayload(void) {
    needToSetAdvPayload = false;
    return transport->getGap().setAdvertisingData(advPayload, scanResponse);
}

inline ble_error_t
BLEDevice::startAdvertising(void)
{
    ble_error_t rc;
    if ((rc = transport->getGattServer().initializeGATTDatabase()) != BLE_ERROR_NONE) {
        return rc;
    }
    if (needToSetAdvPayload) {
        if ((rc = setAdvertisingPayload()) != BLE_ERROR_NONE) {
            return rc;
        }
    }

    return transport->getGap().startAdvertising(advParams);
}

inline ble_error_t
BLEDevice::stopAdvertising(void)
{
    return transport->getGap().stopAdvertising();
}

inline ble_error_t
BLEDevice::setScanParams(uint16_t interval, uint16_t window, uint16_t timeout, bool activeScanning) {
    ble_error_t rc;
    if (((rc = scanningParams.setInterval(interval)) == BLE_ERROR_NONE) &&
        ((rc = scanningParams.setWindow(window))     == BLE_ERROR_NONE) &&
        ((rc = scanningParams.setTimeout(timeout))   == BLE_ERROR_NONE)) {
        scanningParams.setActiveScanning(activeScanning);
        return BLE_ERROR_NONE;
    }

    return rc;
}

inline ble_error_t
BLEDevice::setScanInterval(uint16_t interval) {
    return scanningParams.setInterval(interval);
}

inline ble_error_t
BLEDevice::setScanWindow(uint16_t window) {

    return scanningParams.setWindow(window);
}

inline ble_error_t
BLEDevice::setScanTimeout(uint16_t timeout) {
    return scanningParams.setTimeout(timeout);
}

inline void
BLEDevice::setActiveScan(bool activeScanning) {
    return scanningParams.setActiveScanning(activeScanning);
}

inline ble_error_t
BLEDevice::startScan(void (*callback)(const Gap::AdvertisementCallbackParams_t *params)) {
    return transport->getGap().startScan(scanningParams, callback);
}

template<typename T>
inline ble_error_t
BLEDevice::startScan(T *object, void (T::*memberCallback)(const Gap::AdvertisementCallbackParams_t *params)) {
    return transport->getGap().startScan(scanningParams, object, memberCallback);
}

inline ble_error_t
BLEDevice::stopScan(void) {
    return transport->getGap().stopScan();
}


inline ble_error_t
BLEDevice::disconnect(Gap::DisconnectionReason_t reason)
{
    return transport->getGap().disconnect(reason);
}

inline void
BLEDevice::onTimeout(Gap::EventCallback_t timeoutCallback)
{
    transport->getGap().setOnTimeout(timeoutCallback);
}

inline void
BLEDevice::onConnection(Gap::ConnectionEventCallback_t connectionCallback)
{
    transport->getGap().setOnConnection(connectionCallback);
}

inline void
BLEDevice::onDisconnection(Gap::DisconnectionEventCallback_t disconnectionCallback)
{
    transport->getGap().setOnDisconnection(disconnectionCallback);
}

template<typename T>
inline void
BLEDevice::addToDisconnectionCallChain(T *tptr, void (T::*mptr)(void)) {
    transport->getGap().addToDisconnectionCallChain(tptr, mptr);
}

inline void
BLEDevice::onDataSent(void (*callback)(unsigned count)) {
    transport->getGattServer().setOnDataSent(callback);
}

template <typename T> inline void
BLEDevice::onDataSent(T *objPtr, void (T::*memberPtr)(unsigned count)) {
    transport->getGattServer().setOnDataSent(objPtr, memberPtr);
}

inline void
BLEDevice::onDataWritten(void (*callback)(const GattCharacteristicWriteCBParams *eventDataP)) {
    transport->getGattServer().setOnDataWritten(callback);
}

template <typename T> inline void
BLEDevice::onDataWritten(T *objPtr, void (T::*memberPtr)(const GattCharacteristicWriteCBParams *context)) {
    transport->getGattServer().setOnDataWritten(objPtr, memberPtr);
}

inline ble_error_t
BLEDevice::onDataRead(void (*callback)(const GattCharacteristicReadCBParams *eventDataP)) {
    return transport->getGattServer().setOnDataRead(callback);
}

template <typename T> inline ble_error_t
BLEDevice::onDataRead(T *objPtr, void (T::*memberPtr)(const GattCharacteristicReadCBParams *context)) {
    return transport->getGattServer().setOnDataRead(objPtr, memberPtr);
}

inline void
BLEDevice::onUpdatesEnabled(GattServer::EventCallback_t callback)
{
    transport->getGattServer().setOnUpdatesEnabled(callback);
}

inline void
BLEDevice::onUpdatesDisabled(GattServer::EventCallback_t callback)
{
    transport->getGattServer().setOnUpdatesDisabled(callback);
}

inline void
BLEDevice::onConfirmationReceived(GattServer::EventCallback_t callback)
{
    transport->getGattServer().setOnConfirmationReceived(callback);
}

inline void
BLEDevice::onRadioNotification(Gap::RadioNotificationEventCallback_t callback)
{
    transport->getGap().setOnRadioNotification(callback);
}

inline ble_error_t
BLEDevice::addService(GattService &service)
{
    return transport->getGattServer().addService(service);
}

inline Gap::GapState_t
BLEDevice::getGapState(void) const
{
    return transport->getGap().getState();
}

inline ble_error_t BLEDevice::readCharacteristicValue(GattAttribute::Handle_t attributeHandle, uint8_t *buffer, uint16_t *lengthP)
{
    return transport->getGattServer().readValue(attributeHandle, buffer, lengthP);
}

inline ble_error_t BLEDevice::readCharacteristicValue(Gap::Handle_t connectionHandle, GattAttribute::Handle_t attributeHandle, uint8_t *buffer, uint16_t *lengthP)
{
    return transport->getGattServer().readValue(connectionHandle, attributeHandle, buffer, lengthP);
}

inline ble_error_t
BLEDevice::updateCharacteristicValue(GattAttribute::Handle_t attributeHandle, const uint8_t *value, uint16_t size, bool localOnly)
{
    return transport->getGattServer().updateValue(attributeHandle, const_cast<uint8_t *>(value), size, localOnly);
}

inline ble_error_t
BLEDevice::updateCharacteristicValue(Gap::Handle_t connectionHandle, GattAttribute::Handle_t attributeHandle, const uint8_t *value, uint16_t size, bool localOnly)
{
    return transport->getGattServer().updateValue(connectionHandle, attributeHandle, const_cast<uint8_t *>(value), size, localOnly);
}

inline void
BLEDevice::waitForEvent(void)
{
    transport->waitForEvent();
}

inline ble_error_t
BLEDevice::getPreferredConnectionParams(Gap::ConnectionParams_t *params)
{
    return transport->getGap().getPreferredConnectionParams(params);
}

inline ble_error_t
BLEDevice::setPreferredConnectionParams(const Gap::ConnectionParams_t *params)
{
    return transport->getGap().setPreferredConnectionParams(params);
}

inline ble_error_t
BLEDevice::updateConnectionParams(Gap::Handle_t handle, const Gap::ConnectionParams_t *params) {
    return transport->getGap().updateConnectionParams(handle, params);
}

inline const char *
BLEDevice::getVersion(void)
{
    return transport->getVersion();
}

inline ble_error_t
BLEDevice::setDeviceName(const uint8_t *deviceName)
{
    return transport->getGap().setDeviceName(deviceName);
}

inline ble_error_t
BLEDevice::getDeviceName(uint8_t *deviceName, unsigned *lengthP)
{
    return transport->getGap().getDeviceName(deviceName, lengthP);
}

inline ble_error_t
BLEDevice::setAppearance(uint16_t appearance)
{
    return transport->getGap().setAppearance(appearance);
}

inline ble_error_t
BLEDevice::getAppearance(uint16_t *appearanceP)
{
    return transport->getGap().getAppearance(appearanceP);
}

inline ble_error_t
BLEDevice::setTxPower(int8_t txPower)
{
    return transport->setTxPower(txPower);
}

inline void
BLEDevice::getPermittedTxPowerValues(const int8_t **valueArrayPP, size_t *countP)
{
    transport->getPermittedTxPowerValues(valueArrayPP, countP);
}

inline ble_error_t
BLEDevice::initializeSecurity(bool                          enableBonding,
                              bool                          requireMITM,
                              Gap::SecurityIOCapabilities_t iocaps,
                              const Gap::Passkey_t          passkey)
{
    return transport->initializeSecurity(enableBonding, requireMITM, iocaps, passkey);
}

inline void
BLEDevice::onSecuritySetupInitiated(Gap::SecuritySetupInitiatedCallback_t callback)
{
    transport->getGap().setOnSecuritySetupInitiated(callback);
}

inline void
BLEDevice::onSecuritySetupCompleted(Gap::SecuritySetupCompletedCallback_t callback)
{
    transport->getGap().setOnSecuritySetupCompleted(callback);
}

inline void
BLEDevice::onLinkSecured(Gap::LinkSecuredCallback_t callback)
{
    transport->getGap().setOnLinkSecured(callback);
}

inline void
BLEDevice::onSecurityContextStored(Gap::HandleSpecificEvent_t callback)
{
    transport->getGap().setOnSecurityContextStored(callback);
}

inline void
BLEDevice::onPasskeyDisplay(Gap::PasskeyDisplayCallback_t callback)
{
    return transport->getGap().setOnPasskeyDisplay(callback);
}

inline ble_error_t
BLEDevice::getLinkSecurity(Gap::Handle_t connectionHandle, Gap::LinkSecurityStatus_t *securityStatusP)
{
    return transport->getGap().getLinkSecurity(connectionHandle, securityStatusP);
}

inline ble_error_t
BLEDevice::purgeAllBondingState(void)
{
    return transport->getGap().purgeAllBondingState();
}

#endif // ifndef __BLE_DEVICE__