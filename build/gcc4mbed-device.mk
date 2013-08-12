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
OUTDIR := $(MBED_TARGET_DEVICE)

# Final target binary.  Used for variable target scoping.
TARGET_BIN := $(OUTDIR)/$(PROJECT).bin

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
#LSCRIPT=$(GCC4MBED_DIR)/external/mbed/libraries/mbed/targets/cmsis/TARGET_$(MBED_TARGET_VENDOR)/TARGET_$(MBED_TARGET_DEVICE)/TOOLCHAIN_GCC_ARM/
LSCRIPT=$(GCC4MBED_DIR)/build/mbed.ld

# Location of external library and header dependencies.
EXTERNAL_DIR = $(GCC4MBED_DIR)/external

# Include path which points to external library headers and to subdirectories of this project which contain headers.
SUBDIRS  := $(wildcard $(SRC)/* $(SRC)/*/* $(SRC)/*/*/* $(SRC)/*/*/*/* $(SRC)/*/*/*/*/*)
PROJINCS := $(sort $(dir $(SUBDIRS)))
MBED_DIR := $(EXTERNAL_DIR)/mbed/libraries/mbed
INCDIRS  += $(SRC) $(PROJINCS)
INCDIRS  += $(GCC4MBED_DIR)/mri
INCDIRS  += $(MBED_DIR)/api
INCDIRS  += $(MBED_DIR)/hal
INCDIRS  += $(MBED_DIR)/targets/cmsis
INCDIRS  += $(MBED_DIR)/targets/hal/TARGET_$(MBED_TARGET_VENDOR)/TARGET_$(MBED_TARGET_DEVICE)
INCDIRS  += $(MBED_DIR)/targets/cmsis/TARGET_$(MBED_TARGET_VENDOR)/TARGET_$(MBED_TARGET_DEVICE)

# DEFINEs to be used when building C/C++ code
# UNDONE: Is device specific
DEFINES += -DTARGET_$(MBED_TARGET_DEVICE) -DTARGET_$(MBED_DEVICE) -DTOOLCHAIN_GCC_ARM -DTOOLCHAIN_GCC $(MBED_DEFINES)
DEFINES += -DMRI_ENABLE=$(MRI_ENABLE) -DMRI_INIT_PARAMETERS='"$(MRI_INIT_PARAMETERS)"' 
DEFINES += -DMRI_BREAK_ON_INIT=$(MRI_BREAK_ON_INIT) -DMRI_SEMIHOST_STDIO=$(MRI_SEMIHOST_STDIO)

# Libraries to be linked into final binary
SYS_LIBS  := -lstdc++_s -lsupc++_s -lm -lgcc -lc_s -lgcc -lc_s -lnosys
LIBS      := $(LIBS_PREFIX) 

# Some choices like mbed library and enabling of asserts depend on build type.
ifeq "$(GCC4MBED_TYPE)" "Debug"
MBED_LIBS := $(EXTERNAL_DIR)/mbed/libraries/mbed/Debug/$(MBED_TARGET)/libmbed.a
else
MBED_LIBS := $(EXTERNAL_DIR)/mbed/libraries/mbed/Release/$(MBED_TARGET)/libmbed.a
DEFINES   += -DNDEBUG
endif

ifeq "$(MRI_ENABLE)" "1"
LIBS      += $(GCC4MBED_DIR)/mri/mri.ar
endif

LIBS      += $(MBED_LIBS)
LIBS      += $(LIBS_SUFFIX)

# Compiler flags used to enable creation of header dependencies.
DEPFLAGS := -MMD -MP

# Compiler Options
$(TARGET_BIN): CPP_FLAGS := $(GPFLAGS) -O$(OPTIMIZATION) -g3 $(MBED_TARGET_C_FLAGS) 
$(TARGET_BIN): CPP_FLAGS += -ffunction-sections -fdata-sections  -fno-exceptions -fno-delete-null-pointer-checks
$(TARGET_BIN): CPP_FLAGS += $(patsubst %,-I%,$(INCDIRS))
$(TARGET_BIN): CPP_FLAGS += $(DEFINES)
$(TARGET_BIN): CPP_FLAGS += $(DEPFLAGS)
$(TARGET_BIN): CPP_FLAGS += -Wall -Wextra -Wno-unused-parameter

$(TARGET_BIN): C_FLAGS := $(GCFLAGS) $(CPP_FLAGS)

$(TARGET_BIN): ASM_FLAGS     := -g3 $(MBED_ASM_FLAGS)
$(TARGET_BIN): ASM_GCC_FLAGS := $(AS_GCFLAGS) $(ASM_FLAGS) -x assembler-with-cpp
$(TARGET_BIN): ASM_GCC_FLAGS += $(patsubst %,-I%,$(INCDIRS))
$(TARGET_BIN): ASM_FLAGS     += $(AS_FLAGS)


# Setup wraps for newlib read/writes to redirect to MRI debugger. 
ifeq "$(MRI_ENABLE)" "1"
$(TARGET_BIN): MRI_WRAPS := ,--wrap=_read,--wrap=_write,--wrap=semihost_connected
else
$(TARGET_BIN): MRI_WRAPS :=
endif

# Linker Options.
$(TARGET_BIN): LD_FLAGS := $(MBED_LD_FLAGS) -O$(OPTIMIZATION) -specs=$(GCC4MBED_DIR)/build/startfile.spec
$(TARGET_BIN): LD_FLAGS  += -Wl,-Map=$(OUTDIR)/$(PROJECT).map,--cref,--gc-sections,--wrap=_isatty,--wrap=malloc,--wrap=realloc,--wrap=free$(MRI_WRAPS)
ifneq "$(NO_FLOAT_SCANF)" "1"
$(TARGET_BIN): LD_FLAGS += -u _scanf_float
endif
ifneq "$(NO_FLOAT_PRINTF)" "1"
$(TARGET_BIN): LD_FLAGS += -u _printf_float
endif


#########################################################################
.PHONY: $(MBED_TARGET_DEVICE) $(MBED_TARGET_DEVICE)_clean $(MBED_TARGET_DEVICE)_deploy $(MBED_TARGET_DEVICE)_size

$(MBED_TARGET_DEVICE): $(TARGET_BIN) $(OUTDIR)/$(PROJECT).hex $(OUTDIR)/$(PROJECT).disasm $(MBED_TARGET_DEVICE)_size

$(TARGET_BIN): $(OUTDIR)/$(PROJECT).elf
	@echo Extracting $@
	$(Q) $(OBJCOPY) -O binary $< $@

$(OUTDIR)/$(PROJECT).hex: $(OUTDIR)/$(PROJECT).elf
	@echo Extracting $@
	$(Q) $(OBJCOPY) -R .stack -O ihex $< $@
	
$(OUTDIR)/$(PROJECT).disasm: $(OUTDIR)/$(PROJECT).elf
	@echo Extracting disassembly to $@
	$(Q) $(OBJDUMP) -d -f -M reg-names-std $< >$@
	
$(OUTDIR)/$(PROJECT).elf: $(LSCRIPT) $(OBJECTS) $(LIBS)
	@echo Linking $@
	$(Q) $(LD) $(LD_FLAGS) -T$+ $(SYS_LIBS) -o $@

$(MBED_TARGET_DEVICE)_size: $(OUTDIR)/$(PROJECT).elf
	$(Q) $(SIZE) $<
	@$(BLANK_LINE)

$(MBED_TARGET_DEVICE)_clean: CLEAN_TARGET := $(OUTDIR)
$(MBED_TARGET_DEVICE)_clean: PROJECT      := $(PROJECT)
$(MBED_TARGET_DEVICE)_clean:
	@echo Cleaning $(PROJECT)/$(CLEAN_TARGET)
	$(Q) $(REMOVE_DIR) $(CLEAN_TARGET) $(QUIET)


-include $(DEPFILES)


ifdef LPC_DEPLOY
$(MBED_TARGET_DEVICE)_deploy: DEPLOY_PREFIX := $(OUTDIR)/$(PROJECT)
$(MBED_TARGET_DEVICE)_deploy: $(TARGET_BIN) $(OUTDIR)/$(PROJECT).hex $(OUTDIR)/$(PROJECT).disasm $(MBED_TARGET_DEVICE)_size
	@echo Deploying to target.
	$(Q) $(subst PROJECT,$(DEPLOY_PREFIX),$(LPC_DEPLOY))
endif

#########################################################################
#  Default rules to compile .c and .cpp file to .o
#  and assemble .s files to .o

$(OUTDIR)/gcc4mbed.o : $(GCC4MBED_DIR)/src/gcc4mbed.c makefile
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(C_FLAGS) -c $< -o $@

$(OUTDIR)/%.o : %.cpp makefile
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GPP) $(CPP_FLAGS) -c $< -o $@

$(OUTDIR)/%.o : %.c makefile
	@echo Compiling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(C_FLAGS) -c $< -o $@

$(OUTDIR)/%.o : %.S makefile
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(GCC) $(ASM_GCC_FLAGS) -c $< -o $@

$(OUTDIR)/%.o : %.s makefile
	@echo Assembling $<
	$(Q) $(MKDIR) $(call convert-slash,$(dir $@)) $(QUIET)
	$(Q) $(AS) $(ASM_FLAGS) -o $@ $<

#########################################################################
