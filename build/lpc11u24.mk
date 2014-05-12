# Copyright (C) 2014 - Adam Green (http://mbed.org/users/AdamGreen/)
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
MBED_DEVICE        := LPC11U24
MBED_TARGET        := NXP_LPC11U24
MBED_CLEAN         := $(MBED_DEVICE)-MBED-clean

# Some libraries (mbed and rtos) have device specific source folders.
HAL_TARGET_SRC   := $(MBED_SRC_ROOT)/targets/hal/TARGET_NXP/TARGET_LPC11UXX
HAL_TARGET_SRC   += $(MBED_SRC_ROOT)/targets/hal/TARGET_NXP/TARGET_LPC11UXX/TARGET_LPC11U24_401
CMSIS_TARGET_SRC := $(MBED_SRC_ROOT)/targets/cmsis/TARGET_NXP/TARGET_LPC11UXX
CMSIS_TARGET_SRC += $(MBED_SRC_ROOT)/targets/cmsis/TARGET_NXP/TARGET_LPC11UXX/TOOLCHAIN_GCC_ARM
RTX_TARGET_SRC   := $(GCC4MBED_DIR)/external/mbed/libraries/rtos/rtx/TARGET_M0/TOOLCHAIN_GCC


# Compiler flags which are specifc to this device.
GCC_DEFINES := -DTARGET_LPC11U24 -DTARGET_M0 -DTARGET_NXP -DTARGET_LPC11UXX -DTARGET_LPC11U24_401
GCC_DEFINES += -D__CORTEX_M0 -DARM_MATH_CM0

C_FLAGS   := -mcpu=cortex-m0 -mthumb -mthumb-interwork
ASM_FLAGS := -mcpu=cortex-m0 -mthumb
LD_FLAGS  := -mcpu=cortex-m0 -mthumb


# Linker script to be used.  Indicates what code should be placed where in memory.
LSCRIPT=$(GCC4MBED_DIR)/external/mbed/libraries/mbed/targets/cmsis/TARGET_NXP/TARGET_LPC11UXX/TOOLCHAIN_GCC_ARM/TARGET_LPC11U24_401/LPC11U24.ld


include $(GCC4MBED_DIR)/build/device-common.mk
