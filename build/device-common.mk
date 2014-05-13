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


# Can skip parsing of this makefile if user hasn't requested this device.
ifeq "$(findstring $(MBED_DEVICE),$(DEVICES))" "$(MBED_DEVICE)"


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
GCC_DEFINES += -DTOOLCHAIN_GCC_ARM -DTOOLCHAIN_GCC -D__MBED__=1


# Flags to be used with C/C++ compiler that are shared between Debug and Release builds.
C_FLAGS += -g3 -ffunction-sections -fdata-sections -fno-exceptions -fno-delete-null-pointer-checks -fomit-frame-pointer
C_FLAGS += -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-missing-braces
C_FLAGS += $(GCC_DEFINES)
C_FLAGS += $(DEP_FLAGS)

CPP_FLAGS := $(C_FLAGS) -fno-rtti -std=gnu++11
C_FLAGS   += -std=gnu99


# Flags used to assemble assembly languages sources.
ASM_FLAGS += -g3 -x assembler-with-cpp $(GCC_DEFINES)


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


else
# Have an empty rule for this device since it isn't supported.
.PHONY: $(MBED_DEVICE)

$(MBED_DEVICE):
	@#


endif # ifeq "$(findstring $(MBED_DEVICE),$(DEVICES))"...
