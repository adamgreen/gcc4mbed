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
MBED_DEVICE        := NRF51822
MBED_TARGET        := NORDIC_NRF51822
MBED_CLEAN         := $(MBED_DEVICE)-MBED-clean


# Compiler flags which are specifc to this device.
TARGETS_FOR_DEVICE := TARGET_NRF51822 TARGET_M0 TARGET_NORDIC TARGET_NRF51822_MKIT TARGET_MCU_NRF51822
TARGETS_FOR_DEVICE += TARGET_MCU_NORDIC_16K TARGET_CORTEX_M
GCC_DEFINES := $(patsubst %,-D%,$(TARGETS_FOR_DEVICE))
GCC_DEFINES += -D__CORTEX_M0 -DARM_MATH_CM0 -DNRF51

C_FLAGS   := -mcpu=cortex-m0 -mthumb -mthumb-interwork
ASM_FLAGS := -mcpu=cortex-m0 -mthumb
LD_FLAGS  := -mcpu=cortex-m0 -mthumb


# Extra platform specific object files to link into file binary.
# For NRF51 parts, we add in the softdevice.
DEVICE_OBJECTS := $(MBED_DEVICE)/s130_nrf51_1.0.0_softdevice.o


# Version of MRI library to use for this device.
DEVICE_MRI_LIB :=


# Linker script to be used.  Indicates what code should be placed where in memory.
LSCRIPT=$(GCC4MBED_DIR)/build/NRF51822.ld


include $(GCC4MBED_DIR)/build/device-common.mk


# Rules to build the SoftDevice object file.
$(MBED_DEVICE)/s130_nrf51_1.0.0_softdevice.bin : $(MBED_SRC_ROOT)/targets/hal/TARGET_NORDIC/TARGET_MCU_NRF51822/Lib/s130_nrf51822_1_0_0/s130_nrf51_1.0.0_softdevice.hex
	$(Q) $(OBJCOPY) -I ihex -O binary --gap-fill 0xFF $< $@

$(MBED_DEVICE)/s130_nrf51_1.0.0_softdevice.o : $(MBED_DEVICE)/s130_nrf51_1.0.0_softdevice.bin
	$(Q) $(OBJCOPY) -I binary -O elf32-littlearm -B arm --rename-section .data=.SoftDevice $< $@
