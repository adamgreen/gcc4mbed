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


# Clear out the include path for mbed components to be filled in by the
# components which are actually used according to MBED_LIBS.
MBED_INCLUDES :=


#
# Setup flags that are common across the different pieces of code to be built.
#
# Optimization levels to be used for Debug and Release versions of libraries.
DEBUG_OPTIMIZATION   := 0
RELEASE_OPTIMIZATION := 2


# Compiler flags used to enable creation of header dependency files.
DEP_FLAGS := -MMD -MP


# Preprocessor defines to use when compiling/assembling code with GCC.
GCC_DEFINES := -DTARGET_$(MBED_TARGET_DEVICE) -DTARGET_$(MBED_DEVICE)
GCC_DEFINES += -DTOOLCHAIN_GCC_ARM -DTOOLCHAIN_GCC $(MBED_DEFINES)


# Flags to be used with C/C++ compiler that are shared between Debug and Release builds.
C_FLAGS := -g3 $(MBED_TARGET_C_FLAGS) 
C_FLAGS += -ffunction-sections -fdata-sections -fno-exceptions -fno-delete-null-pointer-checks -fomit-frame-pointer
C_FLAGS += -Wall -Wextra
C_FLAGS += -Wno-unused-parameter -Wno-missing-field-initializers -Wno-missing-braces
C_FLAGS += $(GCC_DEFINES)
C_FLAGS += $(DEP_FLAGS)

CPP_FLAGS := $(C_FLAGS) -fno-rtti -std=gnu++11
C_FLAGS   += -std=gnu99


# Flags used to assemble assembly languages sources.
ASM_FLAGS := -g3 $(MBED_ASM_FLAGS) -x assembler-with-cpp
ASM_FLAGS += $(GCC_DEFINES)


# Include makefiles to build the project and any of the mbed components it
# might require.
include $(GCC4MBED_DIR)/build/main.mk
include $(GCC4MBED_DIR)/build/mbed.mk
include $(GCC4MBED_DIR)/build/rtos.mk
include $(GCC4MBED_DIR)/build/lwip.mk
include $(GCC4MBED_DIR)/build/eth.mk
include $(GCC4MBED_DIR)/build/fs.mk
include $(GCC4MBED_DIR)/build/usbdevice.mk
include $(GCC4MBED_DIR)/build/usbhost.mk
include $(GCC4MBED_DIR)/build/rpc.mk
include $(GCC4MBED_DIR)/build/dsp.mk


# When building the project for this device, use this scoped include path for
# the mbed components used.
$(MBED_DEVICE): MBED_INCLUDES := $(patsubst %,-I%,$(MBED_INCLUDES))
