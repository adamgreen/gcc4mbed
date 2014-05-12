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


# Name of this library.
LIBRARY := mbed


# Directories where source files are found and output files should be placed.
ROOT                 :=$(MBED_SRC_ROOT)
COMMON_SRC           :=$(MBED_SRC_ROOT)/common
API_HEADERS          :=$(MBED_SRC_ROOT)/api
HAL_HEADERS          :=$(MBED_SRC_ROOT)/hal
CMSIS_COMMON_HEADERS :=$(MBED_SRC_ROOT)/targets/cmsis


# Build up list of all C, C++, and Assembly Language files to be compiled/assembled.
HAL_TARGET_SRCS    := $(foreach i,$(HAL_TARGET_SRC),$(wildcard $i/*.c))
CMSIS_TARGET_SRCS  := $(foreach i,$(CMSIS_TARGET_SRC),$(wildcard $i/*.c $i/*.s))
COMMON_SRCS        := $(wildcard $(COMMON_SRC)/*.c)
COMMON_SRCS        += $(wildcard $(COMMON_SRC)/*.cpp)


# Convert list of source files to corresponding list of object files to be generated.
# Debug and Release object files to go into separate sub-directories.
OBJECTS := $(patsubst $(MBED_SRC_ROOT)/%.c,__Output__/%.o,$(HAL_TARGET_SRCS))
OBJECTS += $(patsubst $(MBED_SRC_ROOT)/%,__Output__/%,$(addsuffix .o,$(basename $(CMSIS_TARGET_SRCS))))
OBJECTS += $(patsubst $(MBED_SRC_ROOT)/%.c,__Output__/%.o,$(patsubst $(MBED_SRC_ROOT)/%.cpp,__Output__/%.o,$(COMMON_SRCS)))



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
