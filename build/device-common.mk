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


# Can skip parsing of this makefile if user hasn't requested this device.
ifeq "$(findstring $(MBED_DEVICE),$(DEVICES))" "$(MBED_DEVICE)"


###############################################################################
# Setup flags that are common across the different pieces of code to be built.
###############################################################################
# Optimization levels to be used for Debug and Release versions of libraries.
DEBUG_OPTIMIZATION   := 0
RELEASE_OPTIMIZATION := 2

# Compiler flags used to enable creation of header dependency files.
DEP_FLAGS := -MMD -MP

# Preprocessor defines to use when compiling/assembling code with GCC.
GCC_DEFINES += $(TOOLCHAIN_DEFINES) -D__MBED__=1 -D_C99_SOURCE

# Flags to be used with C/C++ compiler that are shared between Debug and Release builds.
C_FLAGS += -g3 -ffunction-sections -fdata-sections -fno-exceptions -fno-delete-null-pointer-checks -fomit-frame-pointer
C_FLAGS += -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-missing-braces
C_FLAGS += $(GCC_DEFINES)
C_FLAGS += $(DEP_FLAGS)

CPP_FLAGS := $(C_FLAGS) -fno-rtti -std=gnu++11
C_FLAGS   += -std=gnu99

# Flags used to assemble assembly languages sources.
ASM_FLAGS += -g3 -x assembler-with-cpp $(GCC_DEFINES)

# Clear out the include path for the libraries required to build this project.
MBED_INCLUDES :=
LIB_INCLUDES  :=

# Clear out the list of user libraries to link into this binary.
USER_LIBS_FULL :=

# Directories where mbed library output files should be placed.
RELEASE_DIR := $(MBED_RELEASE_DIR)/$(MBED_DEVICE)
DEBUG_DIR   := $(MBED_DEBUG_DIR)/$(MBED_DEVICE)



###############################################################################
# Build User Libraries
###############################################################################
$(foreach i,$(sort $(USER_LIBS)),$(eval $(call build_user_lib,$i)))
$(foreach i,$(USER_LIBS),$(eval $(call add_user_lib,$i)))



###############################################################################
# Build Main Application
###############################################################################
# Output Object Directory.
OUTDIR := $(MBED_DEVICE)

# Final target binary.  Used for variable target scoping.
TARGET_BIN := $(OUTDIR)/$(PROJECT).bin

# Only allow the MRI debug monitor to be enabled for LPC1768 devices.
ifneq "$(DEVICE_MRI_LIB)" ""
    DEVICE_MRI_ENABLE := $(MRI_ENABLE)
else
    DEVICE_MRI_ENABLE := 0
endif

# List of the objects files to be compiled/assembled based on source files in SRC.
OBJECTS := $(call srcs2objs,$(call filter_dirs,$(call recurse_dir,$(SRC)),$(TARGETS_FOR_DEVICE)),$(SRC),$(OUTDIR))
PAT_MATCH = $(foreach v,$(2),$(if $(findstring $(1),$(v)),$(v),))
EXCL_OBJECTS := $(foreach e,$(EXCLUDE),$(call PAT_MATCH,$(e),$(OBJECTS)))
OBJECTS := $(filter-out $(EXCL_OBJECTS),$(OBJECTS))

# Add in the GCC4MBED stubs which allow hooking in the MRI debug monitor.
OBJECTS += $(OUTDIR)/gcc4mbed.o

# Add in device specific object file(s).
OBJECTS += $(DEVICE_OBJECTS)

# Initialize list of the header dependency files, one per object file. Each mbed SDK library will append to this list.
DEPFILES := $(patsubst %.o,%.d,$(OBJECTS))

# Location of external library and header dependencies.
EXTERNAL_DIR = $(GCC4MBED_DIR)/external

# Include path which points to subdirectories of this project, MRI, and user specified directory.
INCLUDE_DIRS := $(patsubst %,-I%,$(INCDIRS) $(SRC) $(call filter_dirs,$(call recurse_dir,$(SRC)),$(TARGETS_FOR_DEVICE)) $(LIB_INCLUDES) $(GCC4MBED_DIR)/mri)

# DEFINEs to be used when building C/C++ code
MAIN_DEFINES := $(DEFINES) -DMRI_ENABLE=$(DEVICE_MRI_ENABLE) -DMRI_INIT_PARAMETERS='"$(MRI_INIT_PARAMETERS)"'
MAIN_DEFINES += -DMRI_BREAK_ON_INIT=$(MRI_BREAK_ON_INIT) -DMRI_SEMIHOST_STDIO=$(MRI_SEMIHOST_STDIO)

# Libraries to be linked into final binary
SYS_LIBS  := -lstdc++ -lsupc++ -lm -lgcc -lc -lgcc -lc -lnosys
LIBS      := $(LIBS_PREFIX) $(USER_LIBS_FULL)

# Some choices like mbed SDK library locations and enabling of asserts depend on build type.
ifeq "$(GCC4MBED_TYPE)" "Debug"
MBED_LIBRARIES := $(patsubst %,$(DEBUG_DIR)/%.a,$(MBED_LIBS))
else
MBED_LIBRARIES := $(patsubst %,$(RELEASE_DIR)/%.a,$(MBED_LIBS))
MAIN_DEFINES   += -DNDEBUG
endif

ifeq "$(DEVICE_MRI_ENABLE)" "1"
LIBS      += $(DEVICE_MRI_LIB)
endif
LIBS      += $(MBED_LIBRARIES)
LIBS      += $(LIBS_SUFFIX)

# Compiler/Assembler options to use when building application for this device.
$(MBED_DEVICE): C_FLAGS   := -O$(OPTIMIZATION) $(C_FLAGS) $(MAIN_DEFINES) $(INCLUDE_DIRS) $(GCFLAGS)
$(MBED_DEVICE): CPP_FLAGS := -O$(OPTIMIZATION) $(CPP_FLAGS) $(MAIN_DEFINES) $(INCLUDE_DIRS) $(GPFLAGS)
$(MBED_DEVICE): ASM_FLAGS := $(ASM_FLAGS) $(GAFLAGS) $(INCLUDE_DIRS)

# Setup wraps for newlib read/writes to redirect to MRI debugger.
ifeq "$(DEVICE_MRI_ENABLE)" "1"
MRI_WRAPS := ,--wrap=_read,--wrap=_write,--wrap=semihost_connected
else
MRI_WRAPS :=
endif

# Linker Options.
$(MBED_DEVICE): LD_FLAGS := $(LD_FLAGS) -specs=$(GCC4MBED_DIR)/build/startfile.spec
$(MBED_DEVICE): LD_FLAGS += -Wl,-Map=$(OUTDIR)/$(PROJECT).map,--cref,--gc-sections,-zmuldefs,--wrap=_isatty,--wrap=malloc,--wrap=realloc,--wrap=free,--wrap=main$(MRI_WRAPS)
ifneq "$(NO_FLOAT_SCANF)" "1"
$(MBED_DEVICE): LD_FLAGS += -u _scanf_float
endif
ifneq "$(NO_FLOAT_PRINTF)" "1"
$(MBED_DEVICE): LD_FLAGS += -u _printf_float
endif
ifeq "$(NEWLIB_NANO)" "1"
$(MBED_DEVICE): LD_FLAGS += -specs=$(GCC4MBED_DIR)/build/nano.specs
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

$(OUTDIR)/$(PROJECT).elf: $(LSCRIPT) $(OBJECTS) $(LIBS)
	@echo Linking $@
	$(Q) $(LD) $(LD_FLAGS) -T$(call all_objs_from_mbed,$+) $(SYS_LIBS) -o $@

$(MBED_DEVICE)-size: $(OUTDIR)/$(PROJECT).elf
	$(Q) $(SIZE) $<
	@$(BLANK_LINE)

$(MBED_DEVICE)-clean: CLEAN_TARGET := $(OUTDIR)
$(MBED_DEVICE)-clean: PROJECT      := $(PROJECT)
$(MBED_DEVICE)-clean:
	@echo Cleaning $(PROJECT)/$(CLEAN_TARGET)
	$(Q) $(REMOVE_DIR) $(CLEAN_TARGET) $(QUIET)
	$(Q) $(REMOVE) $(PROJECT).bin $(QUIET)
	$(Q) $(REMOVE) $(PROJECT).hex $(QUIET)
	$(Q) $(REMOVE) $(PROJECT).elf $(QUIET)

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
# Library mbed.a
###############################################################################
MBED_DIRS := $(call filter_dirs,$(call recurse_dir,$(MBED_SRC_ROOT)),$(TARGETS_FOR_DEVICE))
$(eval $(call build_mbed_lib,mbed,\
                       $(MBED_DIRS),\
                       $(MBED_DIRS)))

###############################################################################
# Library rtos.a
###############################################################################
ifeq "$(findstring rtos,$(MBED_LIBS))" "rtos"
    RTOS_DIRS := $(call filter_dirs,$(call recurse_dir,$(MBED_LIB_SRC_ROOT)/rtos),$(TARGETS_FOR_DEVICE))
    $(eval $(call build_mbed_lib,rtos,$(RTOS_DIRS),$(RTOS_DIRS)))
endif

###############################################################################
# Library net/lwip.a
###############################################################################
ifeq "$(findstring net/lwip,$(MBED_LIBS))" "net/lwip"
    LWIP_DIRS := $(call filter_dirs,$(call recurse_dir,$(MBED_LIB_SRC_ROOT)/net/lwip),$(TARGETS_FOR_DEVICE))
    $(eval $(call build_mbed_lib,net/lwip,$(LWIP_DIRS),$(LWIP_DIRS)))
endif

###############################################################################
# Library net/eth.a
###############################################################################
ifeq "$(findstring net/eth,$(MBED_LIBS))" "net/eth"
    ETH_DIRS := $(call filter_dirs,$(call recurse_dir,$(MBED_LIB_SRC_ROOT)/net/eth),$(TARGETS_FOR_DEVICE))
    $(eval $(call build_mbed_lib,net/eth,$(ETH_DIRS),$(ETH_DIRS)))
endif

###############################################################################
# Library fs.a
###############################################################################
ifeq "$(findstring fs,$(MBED_LIBS))" "fs"
    FS_DIRS := $(call filter_dirs,$(call recurse_dir,$(MBED_LIB_SRC_ROOT)/fs),$(TARGETS_FOR_DEVICE))
    $(eval $(call build_mbed_lib,fs,$(FS_DIRS),$(FS_DIRS)))
endif

###############################################################################
# Library USBDevice.a
###############################################################################
ifeq "$(findstring USBDevice,$(MBED_LIBS))" "USBDevice"
    USB_DEVICE_DIRS := $(call filter_dirs,$(call recurse_dir,$(MBED_LIB_SRC_ROOT)/USBDevice),$(TARGETS_FOR_DEVICE))
    $(eval $(call build_mbed_lib,USBDevice,$(USB_DEVICE_DIRS),$(USB_DEVICE_DIRS)))
endif

###############################################################################
# Library USBHost.a
###############################################################################
ifeq "$(findstring USBHost,$(MBED_LIBS))" "USBHost"
    USB_HOST_DIRS := $(call filter_dirs,$(call recurse_dir,$(MBED_LIB_SRC_ROOT)/USBHost),$(TARGETS_FOR_DEVICE))
    $(eval $(call build_mbed_lib,USBHost,$(USB_HOST_DIRS),$(USB_HOST_DIRS)))
endif

###############################################################################
# Library rpc.a
###############################################################################
ifeq "$(findstring rpc,$(MBED_LIBS))" "rpc"
    RPC_DIRS := $(call filter_dirs,$(call recurse_dir,$(MBED_LIB_SRC_ROOT)/rpc),$(TARGETS_FOR_DEVICE))
    $(eval $(call build_mbed_lib,rpc,$(RPC_DIRS),$(RPC_DIRS)))
endif

###############################################################################
# Library dsp.a
###############################################################################
ifeq "$(findstring dsp,$(MBED_LIBS))" "dsp"
    DSP_DIRS := $(call filter_dirs,$(call recurse_dir,$(MBED_LIB_SRC_ROOT)/dsp),$(TARGETS_FOR_DEVICE))
    $(eval $(call build_mbed_lib,dsp,$(DSP_DIRS),$(DSP_DIRS)))
endif

#########################################################################
#  Default rules to compile c/c++/assembly language sources to objects.
#########################################################################
$(DEBUG_DIR)/%.o : $(MBED_LIB_SRC_ROOT)/%.c
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(C_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(RELEASE_DIR)/%.o : $(MBED_LIB_SRC_ROOT)/%.c
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(C_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(DEBUG_DIR)/%.o : $(MBED_LIB_SRC_ROOT)/%.cpp
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GPP) $(CPP_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(RELEASE_DIR)/%.o : $(MBED_LIB_SRC_ROOT)/%.cpp
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GPP) $(CPP_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(DEBUG_DIR)/%.o : $(MBED_LIB_SRC_ROOT)/%.s
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(ASM_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(RELEASE_DIR)/%.o : $(MBED_LIB_SRC_ROOT)/%.s
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(ASM_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(DEBUG_DIR)/%.o : $(MBED_LIB_SRC_ROOT)/%.S
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(ASM_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(RELEASE_DIR)/%.o : $(MBED_LIB_SRC_ROOT)/%.S
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(ASM_FLAGS) $(MBED_INCLUDES) -c $< -o $@


#########################################################################
# High level rule for cleaning out all official mbed libraries.
#########################################################################
.PHONY: $(MBED_CLEAN)

$(MBED_CLEAN): CLEAN_TARGETS:=$(DEBUG_DIR) $(RELEASE_DIR)
$(MBED_CLEAN):
	@echo Cleaning $(CLEAN_TARGETS)
	$(Q) $(REMOVE_DIR) $(call convert-slash,$(CLEAN_TARGETS)) $(QUIET)


# Pull in all library header dependencies.
-include $(DEPFILES)


# When building the project for this device, use this scoped include path for
# the mbed components used.
$(MBED_DEVICE): MBED_INCLUDES := $(patsubst %,-I%,$(MBED_INCLUDES))

# Do the same for the user libraries.
$(MBED_DEVICE): LIB_INCLUDES  := $(patsubst %,-I%,$(LIB_INCLUDES))


else
# Have an empty rule for this device since it isn't supported.
.PHONY: $(MBED_DEVICE)

ifeq "$(OS)" "Windows_NT"
$(MBED_DEVICE):
	@REM >nul
else
$(MBED_DEVICE):
	@#
endif

endif # ifeq "$(findstring $(MBED_DEVICE),$(DEVICES))"...
