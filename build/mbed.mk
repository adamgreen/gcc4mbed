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


# Name of this library.
LIBRARY := mbed


# Directories where source files are found and output files should be placed.
ROOT                 :=$(GCC4MBED_DIR)/external/mbed/libraries/mbed
HAL_TARGET_SRC       :=$(ROOT)/targets/hal/TARGET_$(MBED_TARGET_VENDOR)/TARGET_$(MBED_TARGET_DEVICE)
CMSIS_TARGET_SRC     :=$(ROOT)/targets/cmsis/TARGET_$(MBED_TARGET_VENDOR)/TARGET_$(MBED_TARGET_DEVICE)
CMSIS_TARGET_TOOL    :=$(ROOT)/targets/cmsis/TARGET_$(MBED_TARGET_VENDOR)/TARGET_$(MBED_TARGET_DEVICE)/TOOLCHAIN_GCC_ARM
COMMON_SRC           :=$(ROOT)/common
API_HEADERS          :=$(ROOT)/api
HAL_HEADERS          :=$(ROOT)/hal
CMSIS_COMMON_HEADERS :=$(ROOT)/targets/cmsis


# Build up list of all C, C++, and Assembly Language files to be compiled/assembled.
HAL_TARGET_SRCS    := $(wildcard $(HAL_TARGET_SRC)/*.c)
CMSIS_TARGET_SRCS  := $(wildcard $(CMSIS_TARGET_SRC)/*.c)
COMMON_SRCS        := $(wildcard $(COMMON_SRC)/*.c)
COMMON_SRCS        += $(wildcard $(COMMON_SRC)/*.cpp)
CMSIS_TARGET_TOOLS := $(wildcard $(CMSIS_TARGET_TOOL)/*.s)


# Convert list of source files to corresponding list of object files to be generated.
# Debug and Release object files to go into separate sub-directories.
OBJECTS := $(patsubst $(ROOT)/%.c,__Output__/%.o,$(HAL_TARGET_SRCS))
OBJECTS += $(patsubst $(ROOT)/%.c,__Output__/%.o,$(CMSIS_TARGET_SRCS))
OBJECTS += $(patsubst $(ROOT)/%.c,__Output__/%.o,$(patsubst $(ROOT)/%.cpp,__Output__/%.o,$(COMMON_SRCS)))
OBJECTS += $(patsubst $(ROOT)/%.s,__Output__/%.o,$(CMSIS_TARGET_TOOLS))


# Include directory list.
PROJINCS := $(API_HEADERS) $(HAL_HEADERS) $(CMSIS_COMMON_HEADERS) $(CMSIS_TARGET_SRC) $(HAL_TARGET_SRC)


include $(GCC4MBED_DIR)/build/lib-common.mk


#########################################################################
# High level rule for cleaning out all official mbed libraries.
#########################################################################
.PHONY: $(MBED_CLEAN)

$(MBED_CLEAN): CLEAN_TARGETS:=$(DEBUG_DIR) $(RELEASE_DIR)
$(MBED_CLEAN):
	@echo Cleaning $(CLEAN_TARGETS)
	$(Q) $(REMOVE_DIR) $(call convert-slash,$(CLEAN_TARGETS)) $(QUIET)
