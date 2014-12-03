# Copyright 2014 Larry Littlefield (https://developer.mbed.org/users/svkatielee/)
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
MBED_DEVICE        := NUCLEO_F411RE
MBED_TARGET        := NUCLEO_F411RE
MBED_CLEAN         := $(MBED_DEVICE)-MBED-clean

# Some libraries (mbed and rtos) have device specific source folders.
HAL_TARGET_SRC   := $(MBED_SRC_ROOT)/targets/hal/TARGET_STM/TARGET_NUCLEO_F411RE
HAL_TARGET_SRC   += $(MBED_SRC_ROOT)/targets/hal/TARGET_STM/TARGET_NUCLEO_F411RE/TARGET_MBED_NUCLEO_F411RE
CMSIS_TARGET_SRC := $(MBED_SRC_ROOT)/targets/cmsis/TARGET_STM/TARGET_NUCLEO_F411RE
CMSIS_TARGET_SRC += $(MBED_SRC_ROOT)/targets/cmsis/TARGET_STM/TARGET_NUCLEO_F411RE/TOOLCHAIN_GCC_ARM
RTX_TARGET_SRC   := $(GCC4MBED_DIR)/external/mbed/libraries/rtos/rtx/TARGET_M4/TOOLCHAIN_GCC


# Compiler flags which are specifc to this device.
GCC_DEFINES := -DTARGET_NUCLEO_F401RE -DTARGET_M4 -DTARGET_STM -DTARGET_STM32F4 -DTARGET_STM32F411RE
GCC_DEFINES += -D__CORTEX_M4 -DARM_MATH_CM4 -D__FPU_PRESENT=1 -DTARGET_FF_ARDUINO -DTARGET_FF_MORPHO

C_FLAGS   := -mcpu=cortex-m4 -mthumb -mthumb-interwork -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
ASM_FLAGS := -mcpu=cortex-m4 -mthumb  -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
LD_FLAGS  := -mcpu=cortex-m4 -mthumb


# Extra platform specific object files to link into file binary.
DEVICE_OBJECTS :=


# Linker script to be used.  Indicates what code should be placed where in memory.
LSCRIPT=$(GCC4MBED_DIR)/external/mbed/libraries/mbed/targets/cmsis/TARGET_STM/TARGET_NUCLEO_F411RE/TOOLCHAIN_GCC_ARM/NUCLEO_F411RE.ld


include $(GCC4MBED_DIR)/build/device-common.mk