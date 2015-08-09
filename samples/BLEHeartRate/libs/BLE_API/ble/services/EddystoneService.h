/* mbed Microcontroller Library
 * Copyright (c) 2006-2015 ARM Limited
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

#ifndef SERVICES_EDDYSTONEBEACON_H_
#define SERVICES_EDDYSTONEBEACON_H_

#include "ble/BLE.h"
#include "mbed.h"

static const uint8_t BEACON_EDDYSTONE[] = {0xAA, 0xFE};

#if 0
#define INFO(MSG, ...) printf("[EddyStone: INFO]" MSG " \t[%s,%d]\r\n", ##__VA_ARGS__, __FILE__, __LINE__);
#define DBG(MSG, ...)  printf("[EddyStone: DBG]"  MSG " \t[%s,%d]\r\n", ##__VA_ARGS__, __FILE__, __LINE__);
#define WARN(MSG, ...) printf("[EddyStone: WARN]" MSG " \t[%s,%d]\r\n", ##__VA_ARGS__, __FILE__, __LINE__);
#define ERR(MSG, ...)  printf("[EddyStone: ERR]"  MSG " \t[%s,%d]\r\n", ##__VA_ARGS__, __FILE__, __LINE__);
#else
#define INFO(MSG, ...)
#define DBG(MSG, ...)
#define WARN(MSG, ...)
#define ERR(MSG, ...)
#endif // if 0

/**
* @class Eddystone
* @brief Eddystone Configuration Service. Can be used to set URL, adjust power levels, and set flags.
* See https://github.com/google/eddystone
*
*/
class EddystoneService
{
public:
    static const unsigned ADVERTISING_INTERVAL_MSEC = 1000; // Advertising interval for config service.
    static const size_t   SERVICE_DATA_MAX          = 31;   // Maximum size of service data in ADV packets

    static const size_t URI_DATA_MAX = 18;
    typedef uint8_t UriData_t[URI_DATA_MAX];

    // UID Frame Type subfields
    static const size_t UID_NAMESPACEID_SIZE = 10;
    static const size_t UID_INSTANCEID_SIZE  = 6;
    typedef uint8_t UIDNamespaceID_t[UID_NAMESPACEID_SIZE];
    typedef uint8_t UIDInstanceID_t[UID_INSTANCEID_SIZE];

    // Eddystone Frame Type ID
    enum {
        FRAME_TYPE_UID = 0x00,
        FRAME_TYPE_URL = 0x10,
        FRAME_TYPE_TLM = 0x20,
        NUM_FRAMETYPES = 3
    };

    static const uint8_t FRAME_SIZE_UID = 20; // includes RFU bytes
    static const uint8_t FRAME_SIZE_TLM = 14; // TLM frame is a constant 14Bytes

public:
    /**
     * Constructor: sets up the parameters used by the Eddystone beacon.
     * This function should be used in lieu of the config service.
     *
     * @param bleIn
     *            ble object for the underlying controller.
     * @oaram interval
     *            This is is how often broadcasts are made (in units of milliseconds).
     * @param txPowerLevel
     *            The broadcasting power level (in dbM).
     * @param UIDNamespaceID
     *            10-Byte Namespace UUID.
     * @param UIDInstanceID
     *            6-Byte Instance UUID.
     * @param url
     *            Shortened URL to broadcast (pass in as a string).
     * @param tlmVersion
     *            version of telemetry data field to use (default to 0x00).
     */
    EddystoneService(BLEDevice     &bleIn,
                     uint16_t       interval       = 100,
                     uint8_t        txPowerLevel   = 0,
                     const uint8_t *UIDNamespaceID = NULL,
                     const uint8_t *UIDInstanceID  = NULL,
                     const char    *url            = NULL,
                     uint8_t        tlmVersion     = 0) :
        ble(bleIn),
        uidIsSet(false),
        urlIsSet(false),
        frameIndex(0),
        uidRFU(0),
        switchFlag(true),
        TlmPduCount(0),
        TlmTimeSinceBoot(0)
    {
        // Check optional frames, set their 'isSet' flags appropriately
        if ((UIDNamespaceID != NULL) & (UIDInstanceID != NULL)) {
            setUIDFrameData(txPowerLevel, UIDNamespaceID, UIDInstanceID);
            uidIsSet = true;
        } else if (url != NULL) {
            setURLFrameData(txPowerLevel, url);
            urlIsSet = true;
        } else {
            // Default TLM frame to version 0x00, start all values at zero to be spec compliant.
            setTLMFrameData(tlmVersion, 0x00, 0x00);
        }

        ble.gap().setTxPower(txPowerLevel);
        ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_NON_CONNECTABLE_UNDIRECTED);
        ble.gap().setAdvertisingInterval(interval);

        ble.gap().clearAdvertisingPayload();
        ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
        ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, BEACON_EDDYSTONE, sizeof(BEACON_EDDYSTONE));

        // Construct TLM Frame in initial advertising.
        uint8_t  serviceData[SERVICE_DATA_MAX];
        unsigned serviceDataLen = 0;
        serviceData[serviceDataLen++] = BEACON_EDDYSTONE[0];
        serviceData[serviceDataLen++] = BEACON_EDDYSTONE[1];
        serviceDataLen               += constructTLMFrame(serviceData + serviceDataLen, SERVICE_DATA_MAX);
        ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::SERVICE_DATA, serviceData, serviceDataLen);

        ble.gap().startAdvertising();
        ble.gap().onRadioNotification(this, &EddystoneService::radioNotificationCallback);

        timeSinceBootTick.attach(this, &EddystoneService::tsbCallback, 0.1); // increment the TimeSinceBoot ticker every 0.1s
    }

    /*
     *  Update the TLM frame battery voltage value
     *  @param[in] voltagemv Voltage to update the TLM field battery voltage with (in mV)
     *  @return nothing
     */
    void updateTlmBatteryVoltage(uint16_t voltagemv) {
        TlmBatteryVoltage = voltagemv;
    }

    /*
     *  Update the TLM frame beacon temperature
     *  @param[in] temp Temperature of beacon (in 8.8fpn)
     *  @return nothing
     */
    void updateTlmBeaconTemp(uint16_t temp) {
        TlmBeaconTemp = temp;
    }

    /*
     *  Update the TLM frame PDU Count field
     *  @param[in] pduCount Number of Advertising frames sent since power-up
     */
    void updateTlmPduCount(uint32_t pduCount) {
        TlmPduCount = pduCount;
    }

    /*
     *  Update the TLM frame Time since boot in 0.1s increments
     *  @param[in] timeSinceBoot Time since boot in 0.1s increments
     */
    void updateTlmTimeSinceBoot(uint32_t timeSinceBoot) {
        TlmTimeSinceBoot = timeSinceBoot;
    }

protected:
    /*
     *  callback function, called every 0.1s, increments the TimeSinceBoot field in the TLM frame
     */
    virtual void tsbCallback(void) {
        TlmTimeSinceBoot++;
    }

private:
    /*
     *  Set Eddystone UID Frame information.
     *
     *  @param[in] power   TX Power in dB measured at 0 meters from the device. Range of -100 to +20 dB.
     *  @param namespaceID 10B namespace ID
     *  @param instanceID  6B instance ID
     *  @param RFU         2B of RFU, initialized to 0x0000 and not broadcast, included for future reference.
     */
    void setUIDFrameData(int8_t power, const UIDNamespaceID_t namespaceID, const UIDInstanceID_t instanceID, uint16_t RFU = 0x0000) {
        if (power > 20) {
            power = 20;
        }
        if (power < -100) {
            power = -100;
        }
        defaultUidPower = power;

        memcpy(defaultUidNamespaceID, namespaceID, UID_NAMESPACEID_SIZE);
        memcpy(defaultUidInstanceID,  instanceID,  UID_INSTANCEID_SIZE);

        uidRFU = (uint16_t)RFU; // this is probably bad form, but it doesn't really matter yet.
    }

    /**
     *  Construct UID frame from private variables
     *
     *  @param[in,out] data
     *                     pointer to array to store constructed frame in
     *  @param[in]     maxSize
     *                     number of bytes left in array, effectively how much empty space is available to write to
     *
     *  @return number of bytes used. negative number indicates error message.
     */
    int constructUIDFrame(uint8_t *data, uint8_t maxSize) {
        if (maxSize < FRAME_SIZE_UID) {
            return -1;
        }

        unsigned index = 0;
        data[index++] = FRAME_TYPE_UID; // 1B  Type

        if (defaultUidPower > 20) { // enforce range of valid values.
            defaultUidPower = 20;
        }
        if (defaultUidPower < -100) {
            defaultUidPower = -100;
        }
        data[index++] = defaultUidPower; // 1B  Power @ 0meter

        for (size_t x = 0; x < UID_NAMESPACEID_SIZE; x++) { // 10B Namespce ID
            data[index++] = defaultUidNamespaceID[x];
        }

        for (size_t x = 0; x< UID_INSTANCEID_SIZE; x++) { // 6B  Instance ID
            data[index++] = defaultUidInstanceID[x];
        }

        if (uidRFU) { // 2B RFU, include if non-zero, otherwise ignore
            data[index++] = (uint8_t)(uidRFU >> 0);
            data[index++] = (uint8_t)(uidRFU >> 8);
        }

        return index;
    }

    /*
     * Update advertising data
     * @return true on success, false on failure
     */
    bool updateAdvPacket(const uint8_t serviceData[], unsigned serviceDataLen) {
        return (ble.gap().updateAdvertisingPayload(GapAdvertisingData::SERVICE_DATA, serviceData, serviceDataLen) == BLE_ERROR_NONE);
    }

    /*
     *  Set Eddystone URL Frame information.
     *  @param[in] power   TX Power in dB measured at 0 meters from the device.
     *  @param url         URL to encode
     *  @return            false on success, true on failure.
     */
    bool setURLFrameData(int8_t power, const char *url) {
        defaultUrlPower = power;
        encodeURL(url, defaultUriData, defaultUriDataLength); // encode URL to URL Formatting
        if (defaultUriDataLength > URI_DATA_MAX) {
            return true; // error, URL is too big
        }
        return false;
    }

    /**
     *  Construct URL frame from private variables.
     *
     *  @param[in,out] data
     *                     pointer to array to store constructed frame in
     *  @param[in]     maxSize
     *                     number of bytes left in array, effectively how much empty space is available to write to.
     *
     *  @return number of bytes used. negative number indicates error message.
     */
    int constructURLFrame(uint8_t *data, uint8_t maxSize) {
        if (maxSize < (1 + 1 + defaultUriDataLength)) {
            return -1;
        }

        unsigned index = 0;
        data[index++] = FRAME_TYPE_URL;                     // 1B  Type
        data[index++] = defaultUrlPower;                    // 1B  TX Power
        for (size_t x = 0; x < defaultUriDataLength; x++) { // 18B of URL Prefix + encoded URL
            data[index++] = defaultUriData[x];
        }

        return index;
    }

    /*
     *  Set Eddystone TLM Frame information.
     *  @param[in] Version    of the TLM beacon data format
     *  @param batteryVoltage in millivolts
     *  @param beaconTemp     in 8.8 floating point notation
     *
     */
    void setTLMFrameData(uint8_t version, uint16_t batteryVoltage, uint16_t beaconTemp, uint32_t pduCount = 0, uint32_t timeSinceBoot = 0) {
        TlmVersion        = version;
        TlmBatteryVoltage = batteryVoltage;
        TlmBeaconTemp     = beaconTemp;
        TlmPduCount       = pduCount; // reset
        TlmTimeSinceBoot  = timeSinceBoot; // reset
    }

    /*
     *  Construct TLM frame from private variables
     *  @param[in,out] data
     *                     pointer to array to store constructed frame in
     *  @param[in]     maxSize
     *                     number of bytes left in array, effectively how much empty space is available to write to.
     *
     *  @return number of bytes used. negative number indicates error message.
     */
    int constructTLMFrame(uint8_t *data, uint8_t maxSize) {
        if (maxSize < FRAME_SIZE_TLM) {
            return -1;
        }

        unsigned index = 0;
        data[index++] = FRAME_TYPE_TLM;                    // Eddystone frame type = Telemetry
        data[index++] = TlmVersion;                        // TLM Version Number
        data[index++] = (uint8_t)(TlmBatteryVoltage >> 8); // Battery Voltage[0]
        data[index++] = (uint8_t)(TlmBatteryVoltage >> 0); // Battery Voltage[1]
        data[index++] = (uint8_t)(TlmBeaconTemp >> 8);     // Beacon Temp[0]
        data[index++] = (uint8_t)(TlmBeaconTemp >> 0);     // Beacon Temp[1]
        data[index++] = (uint8_t)(TlmPduCount >> 24);      // PDU Count [0]
        data[index++] = (uint8_t)(TlmPduCount >> 16);      // PDU Count [1]
        data[index++] = (uint8_t)(TlmPduCount >> 8);       // PDU Count [2]
        data[index++] = (uint8_t)(TlmPduCount >> 0);       // PDU Count [3]
        data[index++] = (uint8_t)(TlmTimeSinceBoot >> 24); // Time Since Boot [0]
        data[index++] = (uint8_t)(TlmTimeSinceBoot >> 16); // Time Since Boot [1]
        data[index++] = (uint8_t)(TlmTimeSinceBoot >> 8);  // Time Since Boot [2]
        data[index++] = (uint8_t)(TlmTimeSinceBoot >> 0);  // Time Since Boot [3]

        return index;
    }

    /*
     *   State machine for switching out frames.
     *   This function is called by the radioNotificationCallback when a frame needs to get swapped out.
     *   This function exists because of time constraints in the radioNotificationCallback, so it is effectively
     *   broken up into two functions.
     */
    void swapOutFrames(void) {
        uint8_t serviceData[SERVICE_DATA_MAX];

        //hard code in the Eddystone UUID
        unsigned serviceDataLen = 0;
        serviceData[serviceDataLen++] = BEACON_EDDYSTONE[0];
        serviceData[serviceDataLen++] = BEACON_EDDYSTONE[1];

        // if certain frames are not enabled, then skip them. Worst case TLM is always enabled
        switch (frameIndex) {
            case 1:
                // URL Frame
                if (urlIsSet) {
                    INFO("Swapping in URL Frame: Power: %d", defaultUrlPower);
                    serviceDataLen += constructURLFrame(serviceData + serviceDataLen, 20);
                    DBG("\t Swapping in URL Frame: len=%d ", serviceDataLen);
                    updateAdvPacket(serviceData, serviceDataLen);
                    switchFlag = false;
                    frameIndex++;
                    break;
                }

            case 2:
                // UID Frame
                if (uidIsSet) {
                    INFO("Swapping in UID Frame: Power: %d", defaultUidPower);
                    serviceDataLen += constructUIDFrame(serviceData + serviceDataLen, 20);
                    DBG("\t Swapping in UID Frame: len=%d", serviceDataLen);
                    updateAdvPacket(serviceData, serviceDataLen);
                    switchFlag = false;
                    frameIndex++;
                    break;
                }

            default:
                // TLM frame
                INFO("Swapping in TLM Frame: version=%x, Batt=%d, Temp = %d, PDUCnt = %d, TimeSinceBoot=%d",
                     TlmVersion,
                     TlmBatteryVoltage,
                     TlmBeaconTemp,
                     TlmPduCount,
                     TlmTimeSinceBoot);
                serviceDataLen += constructTLMFrame(serviceData + serviceDataLen, 20);
                DBG("\t Swapping in TLM Frame: len=%d", serviceDataLen);
                updateAdvPacket(serviceData, serviceDataLen);
                frameIndex++;
                break;
        }
    }

    /*
     *  Callback from onRadioNotification(), used to update the PDUCounter and process next state.
     */
    void radioNotificationCallback(bool radioActive) {
        static const unsigned EDDYSTONE_SWAPFRAME_DELAYMS = 1;

        // Update PDUCount
        TlmPduCount++;
        frameIndex = frameIndex % NUM_FRAMETYPES;

        if (!radioActive) { // false just after a frame is sent
            // state machine to control which packet is being sent
            switch (frameIndex) {
                case 0: // TLM Frame
                    switchFrame.attach_us(this, &EddystoneService::swapOutFrames, EDDYSTONE_SWAPFRAME_DELAYMS);
                    switchFlag = true;
                    break;

                case 1: // URL Frame
                    // switch out packets
                    if (switchFlag) {
                        switchFrame.attach_us(this, &EddystoneService::swapOutFrames, EDDYSTONE_SWAPFRAME_DELAYMS);
                        switchFlag = false;
                    } else {
                        if ((TlmPduCount % 10) == 0) { // every 10 adv packets switch the frame
                            switchFlag = true;
                        }
                    }
                    break;

                case 2: // UIDFrame
                    // switch out packets
                    if (switchFlag) {
                        switchFrame.attach_us(this, &EddystoneService::swapOutFrames, EDDYSTONE_SWAPFRAME_DELAYMS);
                        switchFlag = false;
                    } else {
                        if ((TlmPduCount % 10) == 0) { // every 10 adv packets switch the frame
                            switchFlag = true;
                        }
                    }
                    break;
            }
        }
    }

private:
    BLEDevice         &ble;
    Ticker            timeSinceBootTick;
    Timeout           switchFrame;
    // Default value that is restored on reset
    size_t            defaultUriDataLength;
    UriData_t         defaultUriData;
    UIDNamespaceID_t  defaultUidNamespaceID;
    UIDInstanceID_t   defaultUidInstanceID;
    int8_t            defaultUidPower;
    int8_t            defaultUrlPower;
    unsigned          frameIndex;
    uint16_t          uidRFU;
    bool              uidIsSet;
    bool              urlIsSet;
    bool              switchFlag;

    // Private Variables for Telemetry Data
    uint8_t           TlmVersion;
    volatile uint16_t TlmBatteryVoltage;
    volatile uint16_t TlmBeaconTemp;
    volatile uint32_t TlmPduCount;
    volatile uint32_t TlmTimeSinceBoot;

public:
    /*
     *  Encode a human-readable URI into the binary format defined by URIBeacon spec (https://github.com/google/uribeacon/tree/master/specification).
     */
    static void encodeURL(const char *uriDataIn, UriData_t uriDataOut, size_t &sizeofURIDataOut) {
        const char *prefixes[] = {
            "http://www.",
            "https://www.",
            "http://",
            "https://",
        };
        const size_t NUM_PREFIXES = sizeof(prefixes) / sizeof(char *);
        const char *suffixes[] = {
            ".com/",
            ".org/",
            ".edu/",
            ".net/",
            ".info/",
            ".biz/",
            ".gov/",
            ".com",
            ".org",
            ".edu",
            ".net",
            ".info",
            ".biz",
            ".gov"
        };
        const size_t NUM_SUFFIXES = sizeof(suffixes) / sizeof(char *);

        sizeofURIDataOut = 0;
        memset(uriDataOut, 0, sizeof(UriData_t));

        if ((uriDataIn == NULL) || (strlen(uriDataIn) == 0)) {
            return;
        }

        /*
         * handle prefix
         */
        for (unsigned i = 0; i < NUM_PREFIXES; i++) {
            size_t prefixLen = strlen(prefixes[i]);
            if (strncmp(uriDataIn, prefixes[i], prefixLen) == 0) {
                uriDataOut[sizeofURIDataOut++]  = i;
                uriDataIn                      += prefixLen;
                break;
            }
        }

        /*
         * handle suffixes
         */
        while (*uriDataIn && (sizeofURIDataOut < URI_DATA_MAX)) {
            /* check for suffix match */
            unsigned i;
            for (i = 0; i < NUM_SUFFIXES; i++) {
                size_t suffixLen = strlen(suffixes[i]);
                if (strncmp(uriDataIn, suffixes[i], suffixLen) == 0) {
                    uriDataOut[sizeofURIDataOut++]  = i;
                    uriDataIn                      += suffixLen;
                    break; /* from the for loop for checking against suffixes */
                }
            }
            /* This is the default case where we've got an ordinary character which doesn't match a suffix. */
            if (i == NUM_SUFFIXES) {
                uriDataOut[sizeofURIDataOut++] = *uriDataIn;
                ++uriDataIn;
            }
        }
    }
};

#endif  // SERVICES_EDDYSTONEBEACON_H_