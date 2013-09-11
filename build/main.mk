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


# Output Object Directory.
OUTDIR := $(MBED_DEVICE)

# Final target binary.  Used for variable target scoping.
TARGET_BIN := $(OUTDIR)/$(PROJECT).bin

# Only allow the MRI debug monitor to be enabled for LPC1768 devices.
ifeq "$(MBED_DEVICE)" "LPC1768"
DEVICE_MRI_ENABLE := $(MRI_ENABLE)
else
DEVICE_MRI_ENABLE := 0
endif

# List of sources to be compiled/assembled
C_SRCS   := $(wildcard $(SRC)/*.c $(SRC)/*/*.c $(SRC)/*/*/*.c $(SRC)/*/*/*/*.c $(SRC)/*/*/*/*/*.c)
ASM_SRCS :=  $(wildcard $(SRC)/*.S $(SRC)/*/*.S $(SRC)/*/*/*.S $(SRC)/*/*/*/*.S $(SRC)/*/*/*/*/*.S)
ifneq "$(OS)" "Windows_NT"
ASM_SRCS +=  $(wildcard $(SRC)/*.s $(SRC)/*/*.s $(SRC)/*/*/*.s $(SRC)/*/*/*/*.s $(SRC)/*/*/*/*/*.s)
endif
CPP_SRCS := $(wildcard $(SRC)/*.cpp $(SRC)/*/*.cpp $(SRC)/*/*/*.cpp $(SRC)/*/*/*/*.cpp $(SRC)/*/*/*/*/*.cpp)

# List of the objects files to be compiled/assembled.
OBJECTS := $(patsubst %.c,$(OUTDIR)/%.o,$(C_SRCS)) 
OBJECTS += $(patsubst %.s,$(OUTDIR)/%.o,$(patsubst %.S,$(OUTDIR)/%.o,$(ASM_SRCS)))
OBJECTS += $(patsubst %.cpp,$(OUTDIR)/%.o,$(CPP_SRCS))

# Add in the GCC4MBED stubs which allow hooking in the MRI debug monitor.
OBJECTS += $(OUTDIR)/gcc4mbed.o

# List of the header dependency files, one per object file.
DEPFILES = $(patsubst %.o,%.d,$(OBJECTS))

# Linker script to be used.  Indicates what code should be placed where in memory.
LSCRIPT=$(GCC4MBED_DIR)/external/mbed/libraries/mbed/targets/cmsis/TARGET_$(MBED_TARGET_VENDOR)/TARGET_$(MBED_TARGET_DEVICE)/TOOLCHAIN_GCC_ARM/$(MBED_LD_SCRIPT)

# Location of external library and header dependencies.
EXTERNAL_DIR = $(GCC4MBED_DIR)/external

# Include path which points to external library headers and to subdirectories of this project which contain headers.
SUBDIRS      := $(wildcard $(SRC)/* $(SRC)/*/* $(SRC)/*/*/* $(SRC)/*/*/*/* $(SRC)/*/*/*/*/*)
PROJINCS     := $(sort $(dir $(SUBDIRS)))
INCLUDE_DIRS := $(INCDIRS)
INCLUDE_DIRS += $(SRC) $(PROJINCS)
INCLUDE_DIRS += $(GCC4MBED_DIR)/mri
INCLUDE_DIRS := $(patsubst %,-I%,$(INCLUDE_DIRS))

# DEFINEs to be used when building C/C++ code
DEFINES += -DMRI_ENABLE=$(DEVICE_MRI_ENABLE) -DMRI_INIT_PARAMETERS='"$(MRI_INIT_PARAMETERS)"' 
DEFINES += -DMRI_BREAK_ON_INIT=$(MRI_BREAK_ON_INIT) -DMRI_SEMIHOST_STDIO=$(MRI_SEMIHOST_STDIO)

# Libraries to be linked into final binary
SYS_LIBS  := -lstdc++_s -lsupc++_s -lm -lgcc -lc_s -lgcc -lc_s -lnosys
LIBS      := $(LIBS_PREFIX) 

# Some choices like mbed SDK library locations and enabling of asserts depend on build type.
ifeq "$(GCC4MBED_TYPE)" "Debug"
MBED_LIBRARIES := $(patsubst %,$(EXTERNAL_DIR)/mbed/libraries/Debug/$(MBED_TARGET)/%.a,$(MBED_LIBS))
else
MBED_LIBRARIES := $(patsubst %,$(EXTERNAL_DIR)/mbed/libraries/Release/$(MBED_TARGET)/%.a,$(MBED_LIBS))
DEFINES        += -DNDEBUG
endif

ifeq "$(DEVICE_MRI_ENABLE)" "1"
LIBS      += $(GCC4MBED_DIR)/mri/mri.ar
endif

LIBS      += $(MBED_LIBRARIES)
LIBS      += $(LIBS_SUFFIX)

$(MBED_DEVICE): C_FLAGS   := -O$(OPTIMIZATION) $(C_FLAGS) $(DEFINES) $(INCLUDE_DIRS) $(GCFLAGS)
$(MBED_DEVICE): CPP_FLAGS := -O$(OPTIMIZATION) $(CPP_FLAGS) $(DEFINES) $(INCLUDE_DIRS) $(GPFLAGS)

# Assembler Options.
$(MBED_DEVICE): ASM_FLAGS := $(ASM_FLAGS) $(AS_FLAGS) $(INCLUDE_DIRS)

# Setup wraps for newlib read/writes to redirect to MRI debugger. 
ifeq "$(DEVICE_MRI_ENABLE)" "1"
MRI_WRAPS := ,--wrap=_read,--wrap=_write,--wrap=semihost_connected
else
MRI_WRAPS :=
endif

# Linker Options.
$(MBED_DEVICE): LD_FLAGS := $(MBED_LD_FLAGS) -specs=$(GCC4MBED_DIR)/build/startfile.spec
$(MBED_DEVICE): LD_FLAGS  += -Wl,-Map=$(OUTDIR)/$(PROJECT).map,--cref,--gc-sections,--wrap=_isatty,--wrap=malloc,--wrap=realloc,--wrap=free$(MRI_WRAPS)
ifneq "$(NO_FLOAT_SCANF)" "1"
$(MBED_DEVICE): LD_FLAGS += -u _scanf_float
endif
ifneq "$(NO_FLOAT_PRINTF)" "1"
$(MBED_DEVICE): LD_FLAGS += -u _printf_float
endif


#########################################################################
.PHONY: $(MBED_DEVICE) $(MBED_DEVICE)_clean $(MBED_DEVICE)_deploy $(MBED_DEVICE)_size

$(MBED_DEVICE): $(TARGET_BIN) $(OUTDIR)/$(PROJECT).hex $(OUTDIR)/$(PROJECT).disasm $(MBED_DEVICE)_size

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
	$(Q) $(LD) $(LD_FLAGS) -T$+ $(SYS_LIBS) -o $@

$(MBED_DEVICE)_size: $(OUTDIR)/$(PROJECT).elf
	$(Q) $(SIZE) $<
	@$(BLANK_LINE)

$(MBED_DEVICE)_clean: CLEAN_TARGET := $(OUTDIR)
$(MBED_DEVICE)_clean: PROJECT      := $(PROJECT)
$(MBED_DEVICE)_clean:
	@echo Cleaning $(PROJECT)/$(CLEAN_TARGET)
	$(Q) $(REMOVE_DIR) $(CLEAN_TARGET) $(QUIET)
	$(Q) $(REMOVE) $(PROJECT).bin $(QUIET)
	$(Q) $(REMOVE) $(PROJECT).hex $(QUIET)
	$(Q) $(REMOVE) $(PROJECT).elf $(QUIET)


-include $(DEPFILES)


ifdef LPC_DEPLOY
$(MBED_DEVICE)_deploy: DEPLOY_PREFIX := $(OUTDIR)/$(PROJECT)
$(MBED_DEVICE)_deploy: $(MBED_DEVICE)
	@echo Deploying to target.
	$(Q) $(subst PROJECT,$(DEPLOY_PREFIX),$(LPC_DEPLOY))
endif

#########################################################################
#  Default rules to compile .c and .cpp file to .o
#  and assemble .s files to .o

$(OUTDIR)/gcc4mbed.o : $(GCC4MBED_DIR)/src/gcc4mbed.c makefile
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(C_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(OUTDIR)/%.o : %.cpp makefile
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GPP) $(CPP_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(OUTDIR)/%.o : %.c makefile
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(C_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(OUTDIR)/%.o : %.S makefile
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(ASM_FLAGS) $(MBED_INCLUDES) -c $< -o $@

$(OUTDIR)/%.o : %.S makefile
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(ASM_FLAGS) $(MBED_INCLUDES) -c $< -o $@

#########################################################################
