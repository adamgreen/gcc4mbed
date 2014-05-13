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
