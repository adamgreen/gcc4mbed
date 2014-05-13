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


# Can skip parsing of this makefile if user hasn't requested this library.
ifeq "$(findstring $(LIBRARY),$(MBED_LIBS))" "$(LIBRARY)"


# Directory where source files are found.
ROOT            :=$(GCC4MBED_DIR)/external/mbed/libraries/$(LIBRARY)


# Build up list of all C, C++, and Assembly Language files to be compiled/assembled.
C_SRCS   := $(wildcard $(ROOT)/*.c $(ROOT)/*/*.c $(ROOT)/*/*/*.c $(ROOT)/*/*/*/*.c $(ROOT)/*/*/*/*/*.c)
ASM_SRCS :=  $(wildcard $(ROOT)/*.S $(ROOT)/*/*.S $(ROOT)/*/*/*.S $(ROOT)/*/*/*/*.S $(ROOT)/*/*/*/*/*.S)
ifneq "$(OS)" "Windows_NT"
ASM_SRCS +=  $(wildcard $(ROOT)/*.s $(ROOT)/*/*.s $(ROOT)/*/*/*.s $(ROOT)/*/*/*/*.s $(ROOT)/*/*/*/*/*.s)
endif
CPP_SRCS := $(wildcard $(ROOT)/*.cpp $(ROOT)/*/*.cpp $(ROOT)/*/*/*.cpp $(ROOT)/*/*/*/*.cpp $(ROOT)/*/*/*/*/*.cpp)


# Convert list of source files to corresponding list of object files to be generated.
# Debug and Release object files to go into separate sub-directories.
OBJECTS := $(patsubst $(ROOT)/%.cpp,__Output__/%.o,$(CPP_SRCS))
OBJECTS += $(patsubst $(ROOT)/%.c,__Output__/%.o,$(C_SRCS))
OBJECTS += $(patsubst $(ROOT)/%.s,__Output__/%.o,$(patsubst $(ROOT)/%.S,__Output__/%.o,$(ASM_SRCS)))


# Include path based on all directories in this library.
SUBDIRS  := $(wildcard $(ROOT)/* $(ROOT)/*/* $(ROOT)/*/*/* $(ROOT)/*/*/*/* $(ROOT)/*/*/*/*/*)
PROJINCS := $(sort $(dir $(SUBDIRS)))


include $(GCC4MBED_DIR)/build/lib-common.mk


endif # ifeq "$(findstring $(LIBRARY),$(MBED_LIBS))"...
