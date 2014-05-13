# Copyright 2014 Adam Green (http://mbed.org/users/AdamGreen/)
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
MBED_DEVICE        := KL25Z
MBED_TARGET        := Freescale_KL25Z
MBED_CLEAN         := $(MBED_DEVICE)-MBED-clean

# Some libraries (mbed and rtos) have device specific source folders.
HAL_TARGET_SRC   := $(MBED_SRC_ROOT)/targets/hal/TARGET_Freescale/TARGET_KLXX
HAL_TARGET_SRC   += $(MBED_SRC_ROOT)/targets/hal/TARGET_Freescale/TARGET_KLXX/TARGET_KL25Z
CMSIS_TARGET_SRC := $(MBED_SRC_ROOT)/targets/cmsis/TARGET_Freescale/TARGET_KLXX/TARGET_KL25Z
CMSIS_TARGET_SRC += $(MBED_SRC_ROOT)/targets/cmsis/TARGET_Freescale/TARGET_KLXX/TARGET_KL25Z/TOOLCHAIN_GCC_ARM
RTX_TARGET_SRC   := $(GCC4MBED_DIR)/external/mbed/libraries/rtos/rtx/TARGET_M0P/TOOLCHAIN_GCC


# Compiler flags which are specifc to this device.
GCC_DEFINES := -DTARGET_KL25Z -DTARGET_M0P -DTARGET_Freescale -DTARGET_KLXX
GCC_DEFINES += -D__CORTEX_M0PLUS -DARM_MATH_CM0PLUS

C_FLAGS   := -mcpu=cortex-m0plus -mthumb -mthumb-interwork
ASM_FLAGS := -mcpu=cortex-m0plus -mthumb
LD_FLAGS  := -mcpu=cortex-m0plus -mthumb


# Linker script to be used.  Indicates what code should be placed where in memory.
LSCRIPT=$(GCC4MBED_DIR)/external/mbed/libraries/mbed/targets/cmsis/TARGET_Freescale/TARGET_KLXX/TARGET_KL25Z/TOOLCHAIN_GCC_ARM/MKL25Z4.ld


include $(GCC4MBED_DIR)/build/device-common.mk
