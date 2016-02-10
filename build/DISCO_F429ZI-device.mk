# Copyright 2015 Larry Littlefield (https://developer.mbed.org/users/svkatielee/)
# Copyright 2015 Chang,Jia-Rung    (https://github.com/JaredCJR)
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
MBED_DEVICE        := DISCO_F429ZI
MBED_CLEAN         := $(MBED_DEVICE)-MBED-clean

# Compiler flags which are specifc to this device.
TARGETS_FOR_DEVICE := TARGET_DISCO_F429ZI TARGET_M4 TARGET_RTOS_M4_M7 TARGET_CORTEX_M TARGET_STM TARGET_STM32F4
TARGETS_FOR_DEVICE += TARGET_STM32F429ZI TARGET_FF_MORPHO TARGET_F429ZI
GCC_DEFINES := $(patsubst %,-D%,$(TARGETS_FOR_DEVICE))
GCC_DEFINES += -D__CORTEX_M4 -DARM_MATH_CM4 -D__FPU_PRESENT=1

C_FLAGS   := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb-interwork
ASM_FLAGS := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
LD_FLAGS  := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=softfp

# Extra platform specific object files to link into file binary.
DEVICE_OBJECTS :=


# Version of MRI library to use for this device.
DEVICE_MRI_LIB := $(GCC4MBED_DIR)/mri/libmri_stm32f429-disco.a


# Linker script to be used.  Indicates what code should be placed where in memory.
DISCO_F429ZI_LSCRIPT ?= $(GCC4MBED_DIR)/external/mbed/libraries/mbed/targets/cmsis/TARGET_STM/TARGET_STM32F4/TARGET_DISCO_F429ZI/TOOLCHAIN_GCC_ARM/STM32F429ZI.ld
LSCRIPT = $(DISCO_F429ZI_LSCRIPT)


include $(GCC4MBED_DIR)/build/device-common.mk
