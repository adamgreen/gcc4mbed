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


# Name of library being built.
LIBRARY := rtos


# Can skip parsing of this makefile if user hasn't requested this library.
ifeq "$(findstring $(LIBRARY),$(MBED_LIBS))" "$(LIBRARY)"


# Directories where source files are found and output files should be placed.
ROOT                 :=$(GCC4MBED_DIR)/external/mbed/libraries/$(LIBRARY)
RTOS_SRC             :=$(ROOT)/rtos
RTX_SRC              :=$(ROOT)/rtx


# Build up list of all C, C++, and Assembly Language files to be compiled/assembled.
RTOS_SRCS    := $(wildcard $(RTOS_SRC)/*.cpp)
RTX_SRCS     := $(wildcard $(RTX_SRC)/*.c)
RTX_TARGET_SRCS := $(wildcard $(RTX_TARGET_SRC)/*.s)


# Convert list of source files to corresponding list of object files to be generated.
# Debug and Release object files to go into separate sub-directories.
OBJECTS := $(patsubst $(ROOT)/%.cpp,__Output__/%.o,$(RTOS_SRCS))
OBJECTS += $(patsubst $(ROOT)/%.c,__Output__/%.o,$(RTX_SRCS))
OBJECTS += $(patsubst $(ROOT)/%.s,__Output__/%.o,$(RTX_TARGET_SRCS))


# Include paths to be used for this library.
PROJINCS := $(RTOS_SRC) $(RTX_SRC)


include $(GCC4MBED_DIR)/build/lib-common.mk


endif # ifeq "$(findstring $(LIBRARY),$(MBED_LIBS))"...
