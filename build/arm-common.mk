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


# Build up target library name based on vendor and device name.  This will be
# the root rule for the building of this library.
# NOTE: Most of the variables defined in this included makefile will be scoped
#       to this rule so that they don't interfere with other makefiles.
MBED_TARGET  :=$(MBED_TARGET_VENDOR)_$(MBED_TARGET_DEVICE)
CLEAN_TARGET :=$(MBED_TARGET)_clean


# Directories where source files are found and output files should be placed.
ROOT_DIR             :=$(GCC4MBED_DIR)/external/mbed/libraries/mbed
HAL_TARGET_SRC       :=$(ROOT_DIR)/targets/hal/TARGET_$(MBED_TARGET_VENDOR)/TARGET_$(MBED_TARGET_DEVICE)
CMSIS_TARGET_SRC     :=$(ROOT_DIR)/targets/cmsis/TARGET_$(MBED_TARGET_VENDOR)/TARGET_$(MBED_TARGET_DEVICE)
CMSIS_TARGET_TOOL    :=$(ROOT_DIR)/targets/cmsis/TARGET_$(MBED_TARGET_VENDOR)/TARGET_$(MBED_TARGET_DEVICE)/TOOLCHAIN_GCC_ARM
COMMON_SRC           :=$(ROOT_DIR)/common
API_HEADERS          :=$(ROOT_DIR)/api
HAL_HEADERS          :=$(ROOT_DIR)/hal
CMSIS_COMMON_HEADERS :=$(ROOT_DIR)/targets/cmsis
RELEASE_DIR          :=$(ROOT_DIR)/Release/$(MBED_TARGET)
DEBUG_DIR            :=$(ROOT_DIR)/Debug/$(MBED_TARGET)


# Release and Debug target libraries for C and C++ portions of mbed SDK.
LIB_MBED     := libmbed.a
RELEASE_MBED := $(RELEASE_DIR)/$(LIB_MBED)
DEBUG_MBED   := $(DEBUG_DIR)/$(LIB_MBED)


# Build up list of all C, C++, and Assembly Language files to be compiled/assembled.
HAL_TARGET_SRCS    := $(wildcard $(HAL_TARGET_SRC)/*.c)
CMSIS_TARGET_SRCS  := $(wildcard $(CMSIS_TARGET_SRC)/*.c)
COMMON_SRCS        := $(wildcard $(COMMON_SRC)/*.c)
COMMON_SRCS        += $(wildcard $(COMMON_SRC)/*.cpp)
CMSIS_TARGET_TOOLS := $(wildcard $(CMSIS_TARGET_TOOL)/*.s)


# Convert list of source files to corresponding list of object files to be generated.
# Debug and Release object files to go into separate sub-directories.
OBJECTS := $(patsubst $(ROOT_DIR)/%.c,__Output__/%.o,$(HAL_TARGET_SRCS))
OBJECTS += $(patsubst $(ROOT_DIR)/%.c,__Output__/%.o,$(CMSIS_TARGET_SRCS))
OBJECTS += $(patsubst $(ROOT_DIR)/%.c,__Output__/%.o,$(patsubst $(ROOT_DIR)/%.cpp,__Output__/%.o,$(COMMON_SRCS)))
OBJECTS += $(patsubst $(ROOT_DIR)/%.s,__Output__/%.o,$(CMSIS_TARGET_TOOLS))

DEBUG_OBJECTS   := $(patsubst __Output__%,$(DEBUG_DIR)%,$(OBJECTS))
RELEASE_OBJECTS := $(patsubst __Output__%,$(RELEASE_DIR)%,$(OBJECTS))


# List of the header dependency files, one per object file.
DEBUG_DEPFILES   := $(patsubst %.o,%.d,$(DEBUG_OBJECTS))
RELEASE_DEPFILES := $(patsubst %.o,%.d,$(RELEASE_OBJECTS))


# Include directory list.
INCLUDE_DIRS := $(API_HEADERS) $(HAL_HEADERS) $(CMSIS_COMMON_HEADERS) $(CMSIS_TARGET_SRC) $(HAL_TARGET_SRC)


# Optimization levels to be used for Debug and Release versions of the library.
DEBUG_OPTIMIZATION   := 0
RELEASE_OPTIMIZATION := 2


# Compiler flags used to enable creation of header dependency files.
DEP_FLAGS := -MMD -MP


# Flags to be used with C/C++ compiler that are shared between Debug and Release builds.
C_FLAGS := -g3 $(MBED_TARGET_C_FLAGS) 
C_FLAGS += -ffunction-sections -fdata-sections -fno-exceptions -fno-delete-null-pointer-checks
C_FLAGS += -Wall -Wextra
C_FLAGS += -Wno-unused-parameter
C_FLAGS += $(patsubst %,-I%,$(INCLUDE_DIRS))
C_FLAGS += -DTARGET_$(DEVICE) -DTOOLCHAIN_GCC_ARM -DTOOLCHAIN_GCC
C_FLAGS += $(MBED_DEFINES)
C_FLAGS += $(DEP_FLAGS)

CPP_FLAGS := $(C_FLAGS) -fno-rtti -std=gnu++98
C_FLAGS   += -std=gnu99


# Customize C/C++ flags for Debug and Release builds.
$(DEBUG_MBED): C_FLAGS   := $(C_FLAGS) -O$(DEBUG_OPTIMIZATION)
$(DEBUG_MBED): CPP_FLAGS := $(CPP_FLAGS) -O$(DEBUG_OPTIMIZATION)

$(RELEASE_MBED): C_FLAGS   := $(C_FLAGS) -O$(RELEASE_OPTIMIZATION) -DNDEBUG
$(RELEASE_MBED): CPP_FLAGS := $(CPP_FLAGS) -O$(RELEASE_OPTIMIZATION) -DNDEBUG


# Flags used to assemble assembly languages sources.
ASM_FLAGS := -g3 $(MBED_ASM_FLAGS) -x assembler-with-cpp
ASM_FLAGS += $(patsubst %,-I%,$(INCDIRS))
$(RELEASE_MBED): ASM_FLAGS := $(ASM_FLAGS)
$(DEBUG_MBED):   ASM_FLAGS := $(ASM_FLAGS)


#########################################################################
# High level rules for building Debug and Release versions of library.
#########################################################################
.PHONY: $(CLEAN_TARGET)

$(RELEASE_MBED): $(RELEASE_OBJECTS)
	@echo Linking release library $@
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(AR) -rc $@ $+

$(DEBUG_MBED): $(DEBUG_OBJECTS)
	@echo Linking debug library $@
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(AR) -rc $@ $+

$(CLEAN_TARGET): CLEAN_TARGETS:=$(DEBUG_DIR) $(RELEASE_DIR)
$(CLEAN_TARGET):
	@echo Cleaning $(CLEAN_TARGETS)
	$(Q) $(REMOVE_DIR) $(call convert-slash,$(CLEAN_TARGETS)) $(QUIET)

-include $(DEBUG_DEPFILES)
-include $(RELEASE_DEPFILES)


#########################################################################
#  Default rules to compile c/c++/assembly language sources to objects.
#########################################################################
$(DEBUG_DIR)/%.o : $(ROOT_DIR)/%.c
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(C_FLAGS) -c $< -o $@

$(RELEASE_DIR)/%.o : $(ROOT_DIR)/%.c
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(C_FLAGS) -c $< -o $@

$(DEBUG_DIR)/%.o : $(ROOT_DIR)/%.cpp
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GPP) $(CPP_FLAGS) -c $< -o $@

$(RELEASE_DIR)/%.o : $(ROOT_DIR)/%.cpp
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GPP) $(CPP_FLAGS) -c $< -o $@

$(DEBUG_DIR)/%.o : $(ROOT_DIR)/%.s
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(ASM_FLAGS) -c $< -o $@

$(RELEASE_DIR)/%.o : $(ROOT_DIR)/%.s
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(ASM_FLAGS) -c $< -o $@
