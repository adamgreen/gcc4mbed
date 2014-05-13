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
LIBRARY := net/eth


# Can skip parsing of this makefile if user hasn't requested this library.
ifeq "$(findstring $(LIBRARY),$(MBED_LIBS))" "$(LIBRARY)"


# Directories where source files are found and output files should be placed.
ROOT    :=$(GCC4MBED_DIR)/external/mbed/libraries/$(LIBRARY)
ETH_SRC :=$(ROOT)/EthernetInterface


# Build up list of all C, C++, and Assembly Language files to be compiled/assembled.
ETH_SRCS        := $(wildcard $(ETH_SRC)/*.cpp)
ETH_TARGET_SRCS := $(wildcard $(ETH_TARGET_SRC)/*.c)


# Convert list of source files to corresponding list of object files to be generated.
# Debug and Release object files to go into separate sub-directories.
OBJECTS := $(patsubst $(ROOT)/%.cpp,__Output__/%.o,$(ETH_SRCS))
OBJECTS += $(patsubst $(ROOT)/%.c,__Output__/%.o,$(ETH_TARGET_SRCS))


# Include paths to be used for this library.
PROJINCS := $(ETH_SRC) $(ETH_TARGET_SRC)


include $(GCC4MBED_DIR)/build/lib-common.mk


endif # ifeq "$(findstring $(LIBRARY),$(MBED_LIBS))"...
