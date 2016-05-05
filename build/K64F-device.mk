# Copyright 2016 Adam Green (http://mbed.org/users/AdamGreen/)
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
MBED_DEVICE        := K64F
MBED_CLEAN         := $(MBED_DEVICE)-MBED-clean


# Compiler flags which are specifc to this device.
TARGETS_FOR_DEVICE := TARGET_RTOS_M4_M7 TARGET_FF_ARDUINO TARGET_KPSDK_MCUS TARGET_KPSDK_CODE TARGET_FRDM
TARGETS_FOR_DEVICE += TARGET_CORTEX_M TARGET_Freescale TARGET_M4 TARGET_K64F TARGET_LIKE_CORTEX_M4 TARGET_MCU_K64F
TARGETS_FOR_DEVICE += TARGET_LIKE_MBED
GCC_DEFINES := $(patsubst %,-D%,$(TARGETS_FOR_DEVICE))
GCC_DEFINES += -D__CORTEX_M4 -DARM_MATH_CM4 -D__FPU_PRESENT=1 -DCPU_MK64FN1M0VMD12 -DFSL_RTOS_MBED
C_FLAGS   := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb-interwork
ASM_FLAGS := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
LD_FLAGS  := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=softfp


# Extra platform specific object files to link into file binary.
DEVICE_OBJECTS :=


# Version of MRI library to use for this device.
DEVICE_MRI_LIB :=

# Linker script to be used.  Indicates what code should be placed where in memory.
K64F_LSCRIPT ?= $(GCC4MBED_DIR)/external/mbed/libraries/mbed/targets/cmsis/TARGET_Freescale/TARGET_MCU_K64F/TOOLCHAIN_GCC_ARM/K64FN1M0xxx12.ld
LSCRIPT = $(K64F_LSCRIPT)


include $(GCC4MBED_DIR)/build/device-common.mk
