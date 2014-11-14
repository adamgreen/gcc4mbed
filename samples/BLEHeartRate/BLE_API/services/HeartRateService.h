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

#ifndef __BLE_HEART_RATE_SERVICE_H__
#define __BLE_HEART_RATE_SERVICE_H__

#include "BLEDevice.h"

/* Heart Rate Service */
/* Service:  https://developer.bluetooth.org/gatt/services/Pages/ServiceViewer.aspx?u=org.bluetooth.service.heart_rate.xml */
/* HRM Char: https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.heart_rate_measurement.xml */
/* Location: https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.body_sensor_location.xml */
class HeartRateService {
public:
    enum {
        LOCATION_OTHER = 0,
        LOCATION_CHEST,
        LOCATION_WRIST,
        LOCATION_FINGER,
        LOCATION_HAND,
        LOCATION_EAR_LOBE,
        LOCATION_FOOT,
    };

public:
    /**
     * Constructor.
     *
     * param[in] _ble
     *               Reference to the underlying BLEDevice.
     * param[in] hrmCounter (8-bit)
     *               initial value for the hrm counter.
     * param[in] location
     *               Sensor's location.
     */
    HeartRateService(BLEDevice &_ble, uint8_t hrmCounter, uint8_t location) :
        ble(_ble),
        valueBytes(hrmCounter),
        hrmRate(GattCharacteristic::UUID_HEART_RATE_MEASUREMENT_CHAR, valueBytes.getPointer(),
                valueBytes.getNumValueBytes(), HeartRateValueBytes::MAX_VALUE_BYTES,
                GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
        hrmLocation(GattCharacteristic::UUID_BODY_SENSOR_LOCATION_CHAR, (uint8_t *)&location, sizeof(location), sizeof(location),
                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ),
        controlPoint(GattCharacteristic::UUID_HEART_RATE_CONTROL_POINT_CHAR, (uint8_t *)&controlPointValue,
                     sizeof(controlPointValue), sizeof(controlPointValue), GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE) {
        setupService();
    }

    /**
     * Same constructor as above, but with a 16-bit HRM Counter value.
     */
    HeartRateService(BLEDevice &_ble, uint16_t hrmCounter, uint8_t location) :
        ble(_ble),
        valueBytes(hrmCounter),
        hrmRate(GattCharacteristic::UUID_HEART_RATE_MEASUREMENT_CHAR, valueBytes.getPointer(),
                valueBytes.getNumValueBytes(), HeartRateValueBytes::MAX_VALUE_BYTES,
                GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
        hrmLocation(GattCharacteristic::UUID_BODY_SENSOR_LOCATION_CHAR, (uint8_t *)&location, sizeof(location), sizeof(location),
                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ),
        controlPoint(GattCharacteristic::UUID_HEART_RATE_CONTROL_POINT_CHAR, (uint8_t *)&controlPointValue,
                     sizeof(controlPointValue), sizeof(controlPointValue), GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE) {
        setupService();
    }

    /**
     * Set a new 8-bit value for heart rate.
     */
    void updateHeartRate(uint8_t hrmCounter) {
        valueBytes.updateHeartRate(hrmCounter);
        ble.updateCharacteristicValue(hrmRate.getValueAttribute().getHandle(), valueBytes.getPointer(), valueBytes.getNumValueBytes());
    }

    /**
     * Set a new 16-bit value for heart rate.
     */
    void updateHeartRate(uint16_t hrmCounter) {
        valueBytes.updateHeartRate(hrmCounter);
        ble.updateCharacteristicValue(hrmRate.getValueAttribute().getHandle(), valueBytes.getPointer(), valueBytes.getNumValueBytes());
    }

    /**
     * This callback allows the HeartRateService to receive updates to the
     * controlPoint Characteristic.
     */
    virtual void onDataWritten(const GattCharacteristicWriteCBParams *params) {
        if (params->charHandle == controlPoint.getValueAttribute().getHandle()) {
            /* Do something here if the new value is 1; else you can override this method by
             * extending this class.
             * @NOTE: if you are extending this class, be sure to also call
             * ble.onDataWritten(this, &ExtendedHRService::onDataWritten); in
             * your constructor.
             */
        }
    }

private:
    void setupService(void) {
        static bool serviceAdded = false; /* We should only ever need to add the heart rate service once. */
        if (serviceAdded) {
            return;
        }

        GattCharacteristic *charTable[] = {&hrmRate, &hrmLocation, &controlPoint};
        GattService         hrmService(GattService::UUID_HEART_RATE_SERVICE, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));

        ble.addService(hrmService);
        serviceAdded = true;

        ble.onDataWritten(this, &HeartRateService::onDataWritten);
    }

private:
    /* Private internal representation for the bytes used to work with the vaulue of the heart-rate characteristic. */
    struct HeartRateValueBytes {
        static const unsigned MAX_VALUE_BYTES  = 3; /* FLAGS + up to two bytes for heart-rate */
        static const unsigned FLAGS_BYTE_INDEX = 0;

        static const unsigned VALUE_FORMAT_BITNUM = 0;
        static const uint8_t  VALUE_FORMAT_FLAG   = (1 << VALUE_FORMAT_BITNUM);

        HeartRateValueBytes(uint8_t hrmCounter) : valueBytes() {
            updateHeartRate(hrmCounter);
        }

        HeartRateValueBytes(uint16_t hrmCounter) : valueBytes() {
            updateHeartRate(hrmCounter);
        }

        void updateHeartRate(uint8_t hrmCounter) {
            valueBytes[FLAGS_BYTE_INDEX] &= ~VALUE_FORMAT_FLAG;
            valueBytes[FLAGS_BYTE_INDEX + 1] = hrmCounter;
        }

        void updateHeartRate(uint16_t hrmCounter) {
            valueBytes[FLAGS_BYTE_INDEX] |= VALUE_FORMAT_FLAG;
            valueBytes[FLAGS_BYTE_INDEX + 1] = (uint8_t)(hrmCounter & 0xFF);
            valueBytes[FLAGS_BYTE_INDEX + 2] = (uint8_t)(hrmCounter >> 8);
        }

        uint8_t *getPointer(void) {
            return valueBytes;
        }

        const uint8_t *getPointer(void) const {
            return valueBytes;
        }

        unsigned getNumValueBytes(void) const {
            return 1 + ((valueBytes[FLAGS_BYTE_INDEX] & VALUE_FORMAT_FLAG) ? sizeof(uint16_t) : sizeof(uint8_t));
        }

    private:
        /* First byte = 8-bit values, no extra info, Second byte = uint8_t HRM value */
        /* See --> https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.heart_rate_measurement.xml */
        uint8_t valueBytes[MAX_VALUE_BYTES];
    };

private:
    BLEDevice           &ble;
    HeartRateValueBytes  valueBytes;
    uint8_t              controlPointValue;
    GattCharacteristic   hrmRate;
    GattCharacteristic   hrmLocation;
    GattCharacteristic   controlPoint;
};

#endif /* #ifndef __BLE_HEART_RATE_SERVICE_H__*/
