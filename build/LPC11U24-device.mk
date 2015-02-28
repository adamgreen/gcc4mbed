# Copyright 2015 Adam Green (http://mbed.org/users/AdamGreen/)
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
MBED_DEVICE        := LPC11U24
MBED_TARGET        := NXP_LPC11U24
MBED_CLEAN         := $(MBED_DEVICE)-MBED-clean


# Compiler flags which are specifc to this device.
TARGETS_FOR_DEVICE := TARGET_LPC11U24 TARGET_M0 TARGET_NXP TARGET_LPC11UXX TARGET_LPC11U24_401 TARGET_CORTEX_M
GCC_DEFINES := $(patsubst %,-D%,$(TARGETS_FOR_DEVICE))
GCC_DEFINES += -D__CORTEX_M0 -DARM_MATH_CM0

C_FLAGS   := -mcpu=cortex-m0 -mthumb -mthumb-interwork
ASM_FLAGS := -mcpu=cortex-m0 -mthumb
LD_FLAGS  := -mcpu=cortex-m0 -mthumb


# Extra platform specific object files to link into file binary.
DEVICE_OBJECTS :=


# Version of MRI library to use for this device.
DEVICE_MRI_LIB :=


# Linker script to be used.  Indicates what code should be placed where in memory.
LSCRIPT=$(GCC4MBED_DIR)/external/mbed/libraries/mbed/targets/cmsis/TARGET_NXP/TARGET_LPC11UXX/TOOLCHAIN_GCC_ARM/TARGET_LPC11U24_401/LPC11U24.ld


include $(GCC4MBED_DIR)/build/device-common.mk
