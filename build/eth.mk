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
