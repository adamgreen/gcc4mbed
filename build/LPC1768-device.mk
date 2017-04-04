# Copyright 2017 Adam Green (http://mbed.org/users/AdamGreen/)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Vendor/device for which the library should be built.
MBED_DEVICE        := LPC1768


# Compiler flags which are specifc to this device.
TARGETS_FOR_DEVICE := $(BUILD_TYPE_TARGET) TARGET_LPCTarget TARGET_LPC1768 TARGET_M3 TARGET_CORTEX_M TARGET_LIKE_CORTEX_M3 TARGET_NXP TARGET_LPC176X TARGET_MBED_LPC1768 TARGET_UVISOR_UNSUPPORTED
FEATURES_FOR_DEVICE := FEATURE_LWIP
PERIPHERALS_FOR_DEVICE := DEVICE_ANALOGIN DEVICE_ANALOGOUT DEVICE_CAN DEVICE_DEBUG_AWARENESS DEVICE_ERROR_PATTERN \
                          DEVICE_ETHERNET DEVICE_I2C DEVICE_I2CSLAVE DEVICE_INTERRUPTIN DEVICE_LOCALFILESYSTEM \
                          DEVICE_PORTIN DEVICE_PORTINOUT DEVICE_PORTOUT DEVICE_PWMOUT DEVICE_RTC DEVICE_SEMIHOST \
                          DEVICE_SERIAL DEVICE_SERIAL_FC DEVICE_SLEEP DEVICE_SPI DEVICE_SPISLAVE DEVICE_STDIO_MESSAGES
GCC_DEFINES := $(patsubst %,-D%,$(TARGETS_FOR_DEVICE))
GCC_DEFINES += $(patsubst %,-D%=1,$(FEATURES_FOR_DEVICE))
GCC_DEFINES += $(patsubst %,-D%=1,$(PERIPHERALS_FOR_DEVICE))
GCC_DEFINES += -D__CORTEX_M3 -DARM_MATH_CM3 -D__MBED_CMSIS_RTOS_CM -D__CMSIS_RTOS

C_FLAGS   := -mcpu=cortex-m3 -mthumb
ASM_FLAGS := -mcpu=cortex-m3 -mthumb
LD_FLAGS  := -mcpu=cortex-m3 -mthumb


# Extra platform specific object files to link into file binary.
DEVICE_OBJECTS :=


# Version of MRI library to use for this device.
DEVICE_MRI_LIB := $(GCC4MBED_DIR)/mri/libmri_mbed1768.a


# Determine all mbed source folders which are a match for this device so that it only needs to be done once.
DEVICE_MBED_DIRS := $(call filter_dirs,$(call recurse_dir,$(MBED_SRC_ROOT)),$(TARGETS_FOR_DEVICE),$(FEATURES_FOR_DEVICE))


# Linker script to be used.  Indicates what should be placed where in memory.
LPC1768_LSCRIPT  ?= $(call find_target_linkscript,$(DEVICE_MBED_DIRS))
LSCRIPT          := $(LPC1768_LSCRIPT)


include $(GCC4MBED_DIR)/build/device-common.mk
