# Copyright (C) 2013 - Adam Green (http://mbed.org/users/AdamGreen/)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# Vendor/device for which the library should be built.
MBED_TARGET_VENDOR := Freescale
MBED_TARGET_DEVICE := KL25Z
MBED_DEVICE        := KL25Z
MBED_TARGET_FAMILY := M0P
MBED_TARGET        := $(MBED_TARGET_VENDOR)_$(MBED_TARGET_DEVICE)
MBED_CLEAN         := $(MBED_DEVICE)_MBED_clean


# Compiler flags which are specifc to this device.
MBED_TARGET_C_FLAGS := -mcpu=cortex-m0plus -mthumb
MBED_ASM_FLAGS      := -mcpu=cortex-m0plus -mthumb
MBED_DEFINES        := -D__CORTEX_M0PLUS
MBED_LD_FLAGS       := -mcpu=cortex-m0plus -mthumb


# Linker script to be used for this device.
MBED_LD_SCRIPT      := MKL25Z4.ld


# Clear out the include path for mbed components to be filled in by the
# components which are actually used according to MBED_LIBS.
MBED_INCLUDES :=


# Include makefiles to build the project and any of the mbed components it
# might require.
include $(GCC4MBED_DIR)/build/gcc4mbed-device.mk
include $(GCC4MBED_DIR)/build/mbed-device.mk
include $(GCC4MBED_DIR)/build/rtos-device.mk
include $(GCC4MBED_DIR)/build/lwip-device.mk
include $(GCC4MBED_DIR)/build/eth-device.mk


# When building the project for this device, use this scoped include path for
# the mbed components used.
$(MBED_DEVICE): MBED_INCLUDES := $(patsubst %,-I%,$(MBED_INCLUDES))
