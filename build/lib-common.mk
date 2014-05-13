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


# Directories where output files should be placed.
RELEASE_DIR     :=$(GCC4MBED_DIR)/external/mbed/libraries/Release/$(MBED_TARGET)
DEBUG_DIR       :=$(GCC4MBED_DIR)/external/mbed/libraries/Debug/$(MBED_TARGET)
RELEASE_OBJ_DIR :=$(RELEASE_DIR)/$(LIBRARY)
DEBUG_OBJ_DIR   :=$(DEBUG_DIR)/$(LIBRARY)


# Release and Debug target libraries for C and C++ portions of library.
LIB_NAME     := $(LIBRARY).a
RELEASE_LIB  := $(RELEASE_DIR)/$(LIB_NAME)
DEBUG_LIB    := $(DEBUG_DIR)/$(LIB_NAME)


# Convert list of source files to corresponding list of object files to be generated.
DEBUG_OBJECTS   := $(patsubst __Output__%,$(DEBUG_OBJ_DIR)%,$(OBJECTS))
RELEASE_OBJECTS := $(patsubst __Output__%,$(RELEASE_OBJ_DIR)%,$(OBJECTS))


# List of the header dependency files, one per object file.
DEBUG_DEPFILES   := $(patsubst %.o,%.d,$(DEBUG_OBJECTS))
RELEASE_DEPFILES := $(patsubst %.o,%.d,$(RELEASE_OBJECTS))


# Append to main project's include path.
MBED_INCLUDES += $(ROOT) $(PROJINCS)


# Customize C/C++ flags for Debug and Release builds.
$(DEBUG_LIB): C_FLAGS   := $(C_FLAGS) -O$(DEBUG_OPTIMIZATION)
$(DEBUG_LIB): CPP_FLAGS := $(CPP_FLAGS) -O$(DEBUG_OPTIMIZATION)

$(RELEASE_LIB): C_FLAGS   := $(C_FLAGS) -O$(RELEASE_OPTIMIZATION) -DNDEBUG
$(RELEASE_LIB): CPP_FLAGS := $(CPP_FLAGS) -O$(RELEASE_OPTIMIZATION) -DNDEBUG


# Flags used to assemble assembly languages sources.
$(RELEASE_LIB): ASM_FLAGS := $(ASM_FLAGS)
$(DEBUG_LIB):   ASM_FLAGS := $(ASM_FLAGS)


#########################################################################
# High level rules for building Debug and Release versions of library.
#########################################################################
$(RELEASE_LIB): $(RELEASE_OBJECTS)
	@echo Linking release library $@
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(AR) -rc $@ $+

$(DEBUG_LIB): $(DEBUG_OBJECTS)
	@echo Linking debug library $@
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(AR) -rc $@ $+

-include $(DEBUG_DEPFILES)
-include $(RELEASE_DEPFILES)


#########################################################################
#  Default rules to compile c/c++/assembly language sources to objects.
#########################################################################
$(DEBUG_OBJ_DIR)/%.o : $(ROOT)/%.c
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(C_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(RELEASE_OBJ_DIR)/%.o : $(ROOT)/%.c
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(C_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(DEBUG_OBJ_DIR)/%.o : $(ROOT)/%.cpp
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GPP) $(CPP_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(RELEASE_OBJ_DIR)/%.o : $(ROOT)/%.cpp
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GPP) $(CPP_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(DEBUG_OBJ_DIR)/%.o : $(ROOT)/%.s
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(ASM_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(RELEASE_OBJ_DIR)/%.o : $(ROOT)/%.s
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(ASM_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(DEBUG_OBJ_DIR)/%.o : $(ROOT)/%.S
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(ASM_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(RELEASE_OBJ_DIR)/%.o : $(ROOT)/%.S
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(ASM_FLAGS) $(MBED_INCLUDES) -c $< -o $@
