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
