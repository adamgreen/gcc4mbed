# Copyright 2017 Adam Green (http://mbed.org/users/AdamGreen/)
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


###############################################################################
# Setup flags that are common across the different pieces of code to be built.
###############################################################################
# Preprocessor defines to use when compiling/assembling code with GCC. Append to list from device specific makefile.
GCC_DEFINES += $(TOOLCHAIN_DEFINES) -D_C99_SOURCE

# Preprocessor flags common to multiple source file types (C/C++/ASM/Preprocess/etc).
ALL_DEFINES := -include $(MBED_CONFIG_H) $(GCC_DEFINES) $(MBED_DEFINES)

# Flags to be used with C/C++ compiler that are shared between Debug and Release builds.
C_FLAGS += -g3 -ffunction-sections -fdata-sections -fno-exceptions -fno-delete-null-pointer-checks -fomit-frame-pointer
C_FLAGS += -funsigned-char
C_FLAGS += -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-missing-braces
C_FLAGS += $(ALL_DEFINES)
C_FLAGS += $(DEP_FLAGS)

ifeq "$(NEWLIB_NANO)" "1"
C_FLAGS += -specs=nano.specs
endif

CPP_FLAGS := $(C_FLAGS) -fno-rtti -std=gnu++11 -Wvla
C_FLAGS   += -std=gnu99

# Flags used to assemble assembly languages sources.
ASM_FLAGS += -g3 -x assembler-with-cpp $(ALL_DEFINES)

# Clear out the include path for the libraries required to build this project.
MBED_INCLUDES :=
LIB_INCLUDES  :=

# Clear out the list of user libraries to link into this binary.
USER_LIBS_FULL :=

# Directories where mbed library output files should be placed.
DEBUG_DIR   := $(MBED_DEBUG_DIR)/$(MBED_DEVICE)
DEVELOP_DIR := $(MBED_DEVELOP_DIR)/$(MBED_DEVICE)
RELEASE_DIR := $(MBED_RELEASE_DIR)/$(MBED_DEVICE)



###############################################################################
# Build User Libraries
###############################################################################
DEPFILES :=
$(foreach i,$(sort $(USER_LIBS)),$(eval $(call build_user_lib,$i)))
$(foreach i,$(USER_LIBS),$(eval $(call add_user_lib,$i)))



###############################################################################
# Library mbedos.a source directories
###############################################################################
ifeq "$(MBED_LIB_NAME)" "mbedos"
MBED_DIRS := $(call remove_ignored_dirs,$(DEVICE_MBED_DIRS),$(MBED_IGNORE))
endif

###############################################################################
# Library mbed.a source directories
###############################################################################
ifeq "$(MBED_LIB_NAME)" "mbed"
MBED_DIRS := $(call remove_ignored_dirs,$(DEVICE_MBED_DIRS),$(MBED2_IGNORE))
endif

# Some of the mbed code is included as pre-built binaries licensed under the BPL.
MBED_EXTRA_LIBS := $(subst //,/,$(foreach i,a o,$(foreach j,$(MBED_DIRS),$(wildcard $j/*.$i))))


###############################################################################
# Build Main Application
###############################################################################
# Output Object Directory.
OUTDIR := $(MBED_DEVICE)

# Final target binary.  Used for variable target scoping.
TARGET_BIN := $(OUTDIR)/$(PROJECT).bin

# Only allow the MRI debug monitor to be enabled for devices for which it has support.
ifneq "$(DEVICE_MRI_LIB)" ""
    DEVICE_MRI_ENABLE := $(MRI_ENABLE)
else
    DEVICE_MRI_ENABLE := 0
endif

# List of the objects files to be compiled/assembled based on source files in SRC.
MAIN_DIRS := $(call filter_dirs,$(RAW_MAIN_DIRS),$(TARGETS_FOR_DEVICE),$(FEATURES_FOR_DEVICE))
OBJECTS := $(call srcs2objs,$(MAIN_DIRS),$(SRC),$(OUTDIR))
PAT_MATCH = $(foreach v,$(2),$(if $(findstring $(1),$(v)),$(v),))
EXCL_OBJECTS := $(foreach e,$(EXCLUDE),$(call PAT_MATCH,$(e),$(OBJECTS)))
OBJECTS := $(filter-out $(EXCL_OBJECTS),$(OBJECTS))

# Add in the GCC4MBED stubs which allow hooking in the MRI debug monitor plus other GCC4MBED customization.
OBJECTS += $(OUTDIR)/gcc4mbed.o

# Add in device specific object file(s).
OBJECTS += $(DEVICE_OBJECTS)

# Initialize list of the header dependency files, one per object file. Each mbed SDK library will append to this list.
DEPFILES += $(patsubst %.o,%.d,$(OBJECTS))

# Include path which points to subdirectories of this project, MRI, and user specified directory.
INCLUDE_DIRS := $(patsubst %,-I%,$(INCDIRS) $(MAIN_DIRS) $(LIB_INCLUDES) $(GCC4MBED_DIR)/mri)

# DEFINEs to be used when building main application's C/C++ code
MAIN_DEFINES := $(DEFINES) -DMRI_ENABLE=$(DEVICE_MRI_ENABLE) -DMRI_INIT_PARAMETERS='"$(MRI_INIT_PARAMETERS)"'
MAIN_DEFINES += -DMRI_BREAK_ON_INIT=$(MRI_BREAK_ON_INIT) -DMRI_SEMIHOST_STDIO=$(MRI_SEMIHOST_STDIO)

# Libraries to be linked into final binary
SYS_LIBS  := -lstdc++ -lsupc++ -lm -lgcc -lc -lgcc -lc -lnosys
LIBS      := $(LIBS_PREFIX) $(USER_LIBS_FULL)

# mbed library locations depend on build type.
ifeq "$(GCC4MBED_TYPE)" "Debug"
MBED_LIBRARIES := $(patsubst %,$(DEBUG_DIR)/%.a,$(MBED_LIBS))
endif
ifeq "$(GCC4MBED_TYPE)" "Develop"
MBED_LIBRARIES := $(patsubst %,$(DEVELOP_DIR)/%.a,$(MBED_LIBS))
endif
ifeq "$(GCC4MBED_TYPE)" "Release"
MBED_LIBRARIES := $(patsubst %,$(RELEASE_DIR)/%.a,$(MBED_LIBS))
endif

ifeq "$(DEVICE_MRI_ENABLE)" "1"
LIBS += $(DEVICE_MRI_LIB)
endif
LIBS += $(MBED_LIBRARIES)
LIBS += $(LIBS_SUFFIX)

# Compiler/Assembler options to use when building application for this device.
$(MBED_DEVICE): C_FLAGS     := -O$(OPTIMIZATION) $(C_FLAGS) $(MAIN_DEFINES) $(INCLUDE_DIRS) $(GCFLAGS)
$(MBED_DEVICE): CPP_FLAGS   := -O$(OPTIMIZATION) $(CPP_FLAGS) $(MAIN_DEFINES) $(INCLUDE_DIRS) $(GPFLAGS)
$(MBED_DEVICE): ASM_FLAGS   := $(ASM_FLAGS) $(MAIN_DEFINES) $(INCLUDE_DIRS) $(GAFLAGS)
$(MBED_DEVICE): ALL_DEFINES := $(ALL_DEFINES)

# Setup wraps for newlib read/writes to redirect to MRI debugger.
ifeq "$(DEVICE_MRI_ENABLE)" "1"
MRI_WRAPS := ,--wrap=_read,--wrap=_write,--wrap=semihost_connected,--wrap=semihost_disabledebug
else
MRI_WRAPS :=
endif

# Setup wraps for newlib provided by src/gcc4mbed.c
GCC4MBED_WRAPS := ,--wrap=_isatty

# Setup wraps for newlib provided by mbed.
MBED_WRAPS := ,--wrap=_malloc_r,--wrap=_free_r,--wrap=_realloc_r,--wrap=_calloc_r,--wrap=main,--wrap=exit


# Linker Options.
$(MBED_DEVICE): LD_FLAGS := $(LD_FLAGS) -specs=$(GCC4MBED_DIR)/build/startfile.spec
$(MBED_DEVICE): LD_FLAGS += -Wl,-Map=$(OUTDIR)/$(PROJECT).map,--cref,--gc-sections,-zmuldefs$(GCC4MBED_WRAPS)$(MBED_WRAPS)$(MRI_WRAPS)

ifneq "$(NO_FLOAT_SCANF)" "1"
$(MBED_DEVICE): LD_FLAGS += -u _scanf_float
endif

ifneq "$(NO_FLOAT_PRINTF)" "1"
$(MBED_DEVICE): LD_FLAGS += -u _printf_float
endif

ifeq "$(NEWLIB_NANO)" "1"
$(MBED_DEVICE): LD_FLAGS  += -specs=nano.specs
endif


.PHONY: $(MBED_DEVICE) $(MBED_DEVICE)-clean $(MBED_DEVICE)-deploy $(MBED_DEVICE)-size

$(MBED_DEVICE): $(TARGET_BIN) $(OUTDIR)/$(PROJECT).hex $(OUTDIR)/$(PROJECT).disasm $(MBED_DEVICE)-size

$(TARGET_BIN): $(OUTDIR)/$(PROJECT).elf
	@echo Extracting $@
	$(Q) $(OBJCOPY) -O binary $< $@

$(OUTDIR)/$(PROJECT).hex: $(OUTDIR)/$(PROJECT).elf
	@echo Extracting $@
	$(Q) $(OBJCOPY) -R .stack -O ihex $< $@

$(OUTDIR)/$(PROJECT).disasm: $(OUTDIR)/$(PROJECT).elf
	@echo Extracting disassembly to $@
	$(Q) $(OBJDUMP) -d -f -M reg-names-std --demangle $< >$@

$(OUTDIR)/$(PROJECT).elf: $(OUTDIR)/$(PROJECT).ld $(OBJECTS) $(LIBS) $(MBED_EXTRA_LIBS)
	@echo Linking $@
	$(Q) $(LD) $(LD_FLAGS) -T$(call all_objs_from_mbed,$+) $(SYS_LIBS) -o $@

$(OUTDIR)/$(PROJECT).ld: $(LSCRIPT)
	@echo Preprocessing $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) -E -P  $(ALL_DEFINES) -x c $< -o $@

$(MBED_DEVICE)-size: $(OUTDIR)/$(PROJECT).elf
	$(Q) $(SIZE) $<
	-@echo ''

$(MBED_DEVICE)-clean: CLEAN_TARGET := $(OUTDIR)
$(MBED_DEVICE)-clean: PROJECT      := $(PROJECT)
$(MBED_DEVICE)-clean:
	@echo Cleaning $(PROJECT)/$(CLEAN_TARGET)
	$(Q) $(REMOVE_DIR) $(CLEAN_TARGET) $(QUIET)

ifdef GCC4MBED_DEPLOY
$(MBED_DEVICE)-deploy: DEPLOY_PREFIX := $(OUTDIR)/$(PROJECT)
$(MBED_DEVICE)-deploy: $(MBED_DEVICE)
	@echo Deploying to target.
	$(Q) $(subst PROJECT,$(call convert-slash,$(DEPLOY_PREFIX)),$(GCC4MBED_DEPLOY))
endif

$(OUTDIR)/gcc4mbed.o : $(GCC4MBED_DIR)/src/gcc4mbed.c $(firstword $(MAKEFILE_LIST))
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(C_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(OUTDIR)/%.o : $(SRC)/%.cpp  $(firstword $(MAKEFILE_LIST))
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GPP) $(CPP_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(OUTDIR)/%.o : $(SRC)/%.c  $(firstword $(MAKEFILE_LIST))
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(C_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(OUTDIR)/%.o : $(SRC)/%.S  $(firstword $(MAKEFILE_LIST))
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(ASM_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(OUTDIR)/%.o : $(SRC)/%.s  $(firstword $(MAKEFILE_LIST))
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(ASM_FLAGS) $(MBED_INCLUDES) -c $< -o $@


###############################################################################
# Library mbedos.a
###############################################################################
ifeq "$(MBED_LIB_NAME)" "mbedos"
$(eval $(call build_mbed_lib,mbedos,\
                       $(MBED_DIRS),\
                       $(MBED_SRC_ROOT) $(MBED_DIRS)))
endif

###############################################################################
# Library mbed.a
###############################################################################
ifeq "$(MBED_LIB_NAME)" "mbed"
$(eval $(call build_mbed_lib,mbed,\
                       $(MBED_DIRS),\
                       $(MBED_SRC_ROOT) $(MBED_DIRS)))
endif


#########################################################################
#  Default rules to compile c/c++/assembly language sources to objects.
#########################################################################
$(DEBUG_DIR)/%.o : $(MBED_SRC_ROOT)/%.c
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(C_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(DEBUG_DIR)/%.o : $(MBED_SRC_ROOT)/%.cpp
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GPP) $(CPP_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(DEBUG_DIR)/%.o : $(MBED_SRC_ROOT)/%.s
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(ASM_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(DEBUG_DIR)/%.o : $(MBED_SRC_ROOT)/%.S
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(ASM_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(DEVELOP_DIR)/%.o : $(MBED_SRC_ROOT)/%.c
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(C_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(DEVELOP_DIR)/%.o : $(MBED_SRC_ROOT)/%.cpp
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GPP) $(CPP_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(DEVELOP_DIR)/%.o : $(MBED_SRC_ROOT)/%.s
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(ASM_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(DEVELOP_DIR)/%.o : $(MBED_SRC_ROOT)/%.S
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(ASM_FLAGS) $(MBED_INCLUDES) -c $< -o $@
$(RELEASE_DIR)/%.o : $(MBED_SRC_ROOT)/%.c
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(C_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(RELEASE_DIR)/%.o : $(MBED_SRC_ROOT)/%.cpp
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GPP) $(CPP_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(RELEASE_DIR)/%.o : $(MBED_SRC_ROOT)/%.s
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(ASM_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(RELEASE_DIR)/%.o : $(MBED_SRC_ROOT)/%.S
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(ASM_FLAGS) $(MBED_INCLUDES) -c $< -o $@


# Pull in all library header dependencies.
-include $(DEPFILES)


# When building the project for this device, use this scoped include path for
# the mbed components used.
$(MBED_DEVICE): MBED_INCLUDES := $(patsubst %,-I%,$(MBED_INCLUDES))

# Do the same for the user libraries.
$(MBED_DEVICE): LIB_INCLUDES  := $(patsubst %,-I%,$(LIB_INCLUDES))
