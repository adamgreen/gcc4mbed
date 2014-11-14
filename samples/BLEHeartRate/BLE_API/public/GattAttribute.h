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


#ifndef __GATT_ATTRIBUTE_H__
#define __GATT_ATTRIBUTE_H__

#include "blecommon.h"
#include "UUID.h"

/**************************************************************************/
/*!
    \brief  GATT attribute
*/
/**************************************************************************/
class GattAttribute
{
public:
    typedef uint16_t Handle_t;

public:
    /**
     *  @brief  Creates a new GattAttribute using the specified
     *          UUID, value length, and inital value
     *
     *  @param[in]  uuid
     *              The UUID to use for this attribute
     *  @param[in]  valuePtr
     *              The memory holding the initial value.
     *  @param[in]  initialLen
     *              The min length in bytes of this characteristic's value
     *  @param[in]  maxLen
     *              The max length in bytes of this characteristic's value
     *
     *  @section EXAMPLE
     *
     *  @code
     *
     *  // UUID = 0x2A19, Min length 2, Max len = 2, Properties = write
     *  GattCharacteristic c = GattCharacteristic( 0x2A19, 2, 2, BLE_GATT_CHAR_PROPERTIES_WRITE );
     *
     *  @endcode
     */
    /**************************************************************************/
    GattAttribute(const UUID &uuid, uint8_t *valuePtr = NULL, uint16_t initialLen = 0, uint16_t maxLen = 0) :
        _uuid(uuid), _valuePtr(valuePtr), _initialLen(initialLen), _lenMax(maxLen), _handle(){
        /* empty */
    }

public:
    Handle_t getHandle(void) const {
        return _handle;
    }

    void setHandle(Handle_t id) {
        _handle = id;
    }

    const UUID &getUUID(void) const {
        return _uuid;
    }

    uint16_t getInitialLength(void) const {
        return _initialLen;
    }

    uint16_t getMaxLength(void) const {
        return _lenMax;
    }

    uint8_t *getValuePtr(void) {
        return _valuePtr;
    }

protected:
    UUID      _uuid;        /* Characteristic UUID */
    uint8_t  *_valuePtr;
    uint16_t  _initialLen;  /* Initial length of the value */
    uint16_t  _lenMax;      /* Maximum length of the value */
    Handle_t  _handle;
};

#endif // ifndef __GATT_ATTRIBUTE_H__
