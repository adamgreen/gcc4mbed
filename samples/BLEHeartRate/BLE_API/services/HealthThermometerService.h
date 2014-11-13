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

#ifndef __BLE_HEALTH_THERMOMETER_SERVICE_H__
#define __BLE_HEALTH_THERMOMETER_SERVICE_H__

#include "BLEDevice.h"

/* Health Thermometer Service */
/* Service:  https://developer.bluetooth.org/gatt/profiles/Pages/ProfileViewer.aspx?u=org.bluetooth.profile.health_thermometer.xml */
/* Temperature Measurement: https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.temperature_measurement.xml */
/* Temperature Type: https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.temperature_type.xml */
class HealthThermometerService {
public:
    enum {
        LOCATION_ARMPIT = 1,
        LOCATION_BODY,
        LOCATION_EAR,
        LOCATION_FINGER,
        LOCATION_GI_TRACT,
        LOCATION_MOUTH,
        LOCATION_RECTUM,
        LOCATION_TOE,
        LOCATION_EAR_DRUM,
    };

public:

    /**
     * @param[in] _ble         reference to the BLE device
     * @param[in] initialTemp  initial value in celsius
     * @param[in] _location
     */
    HealthThermometerService(BLEDevice &_ble, float initialTemp, uint8_t _location) :
        ble(_ble),
        valueBytes(initialTemp),
        tempMeasurement(GattCharacteristic::UUID_TEMPERATURE_MEASUREMENT_CHAR, valueBytes.getPointer(),
                        sizeof(TemperatureValueBytes), sizeof(TemperatureValueBytes),
                        GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
        tempLocation(GattCharacteristic::UUID_TEMPERATURE_TYPE_CHAR, (uint8_t *)&_location, sizeof(_location), sizeof(_location),
                     GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ) {

        GattCharacteristic *hrmChars[] = {&tempMeasurement, &tempLocation, };
        GattService         hrmService(GattService::UUID_HEALTH_THERMOMETER_SERVICE, hrmChars, sizeof(hrmChars) / sizeof(GattCharacteristic *));

        ble.addService(hrmService);
    }

    void updateTemperature(float temperature) {
        if (ble.getGapState().connected) {
            valueBytes.updateTemperature(temperature);
            ble.updateCharacteristicValue(tempMeasurement.getValueAttribute().getHandle(), valueBytes.getPointer(), sizeof(TemperatureValueBytes));
        }
    }

private:
    /* Private internal representation for the bytes used to work with the vaulue of the heart-rate characteristic. */
    struct TemperatureValueBytes {
        static const unsigned OFFSET_OF_FLAGS    = 0;
        static const unsigned OFFSET_OF_VALUE    = OFFSET_OF_FLAGS + sizeof(uint8_t);
        static const unsigned SIZEOF_VALUE_BYTES = sizeof(uint8_t) + sizeof(float);

        static const unsigned TEMPERATURE_UNITS_FLAG_POS = 0;
        static const unsigned TIMESTAMP_FLAG_POS         = 1;
        static const unsigned TEMPERATURE_TYPE_FLAG_POS  = 2;

        static const uint8_t TEMPERATURE_UNITS_CELSIUS    = 0;
        static const uint8_t TEMPERATURE_UNITS_FAHRENHEIT = 1;

        TemperatureValueBytes(float initialTemperature) : bytes() {
            /* assumption: temperature values are expressed in Celsius */
            bytes[OFFSET_OF_FLAGS] =  (TEMPERATURE_UNITS_CELSIUS << TEMPERATURE_UNITS_FLAG_POS) |
                                      (false << TIMESTAMP_FLAG_POS) |
                                      (false << TEMPERATURE_TYPE_FLAG_POS);
            updateTemperature(initialTemperature);
        }

        void updateTemperature(float temp) {
            uint32_t temp_ieee11073 = quick_ieee11073_from_float(temp);
            memcpy(&bytes[OFFSET_OF_VALUE], &temp_ieee11073, sizeof(float));
        }

        uint8_t *getPointer(void) {
            return bytes;
        }

        const uint8_t *getPointer(void) const {
            return bytes;
        }

    private:
        /**
         * @brief A very quick conversion between a float temperature and 11073-20601 FLOAT-Type.
         * @param temperature The temperature as a float.
         * @return The temperature in 11073-20601 FLOAT-Type format.
         */
        uint32_t quick_ieee11073_from_float(float temperature) {
            uint8_t  exponent = 0xFE; //exponent is -2
            uint32_t mantissa = (uint32_t)(temperature * 100);

            return (((uint32_t)exponent) << 24) | mantissa;
        }


    private:
        /* First byte = 8-bit flags, Second field is a float holding the temperature value. */
        /* See --> https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.temperature_measurement.xml */
        uint8_t bytes[SIZEOF_VALUE_BYTES];
    };

private:
    BLEDevice             &ble;
    TemperatureValueBytes  valueBytes;
    GattCharacteristic     tempMeasurement;
    GattCharacteristic     tempLocation;
};

#endif /* #ifndef __BLE_HEALTH_THERMOMETER_SERVICE_H__*/
