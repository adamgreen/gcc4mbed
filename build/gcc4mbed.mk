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
###############################################################################
# USAGE:
# Variables that must be defined in including makefile.
#   PROJECT: Name to be given to the output binary for this project.
#   GCC4MBED_DIR: The root directory for where the gcc4mbed sources are located
#                 in your project.  This should point to the parent directory
#                 of the build directory which contains this gcc4mbed.mk file.
#
# Variables that may be optionally set in makefile.
#   SRC: The root directory for the sources of your project.  Defaults to '.'.
#   LIBS_PREFIX: List of library/object files to prepend to mbed.ar capi.ar libs.
#   LIBS_SUFFIX: List of library/object files to append to mbed.ar capi.ar libs.
#   GCC4MBED_TYPE: Type of build to produce.  Allowed values are:
#                  Debug - Build for debugging.  Disables optimizations and
#                          links in debug MRI runtime.  Best debugging 
#                          experience.
#                  Release - Build for release with no debug support.
#                  Checked - Release build with debug support.  Due to
#                            optimizations, debug experience won't be as good
#                            as Debug but might be needed when bugs don't
#                            reproduce in Debug builds.
#                  default: Release
#   GPFLAGS: Additional compiler flags used when building C++ sources.
#   GCFLAGS: Additional compiler flags used when building C sources.
#   AS_GCFLAGS: Additional compiler flags used by GCC when building
#               preprocessed assembly language sources.
#   AS_FLAGS: Additional assembler flags used when building assembly language
#             sources.
#   NO_FLOAT_SCANF: When set to 1, scanf() will not support %f specifier to
#                   input floating point values.  Reduces code size.
#   NO_FLOAT_PRINTF: When set to 1, scanf() will not support %f specifier to
#                    output floating point values.  Reduces code size.
#   VERBOSE: When set to 1, all build commands will be displayed to console.
#            It defaults to 0 which suppresses the output of the build tool
#            command lines themselves.
#   MRI_BREAK_ON_INIT: Should the program halt before calling into main(),
#                      allowing the developer time to set breakpoints in main()
#                      or in code run from within global constructors.
#                      default: 1 - break on init.
#   MRI_SEMIHOST_STDIO: Set to non-zero value to allow debug monitor to use
#                       semi-host calls to redirect stdin/stdout/stderr to the
#                       gdb console.
#                       default: 1 for Debug/Checked builds and 0 for Release.
#   MRI_UART: Select the UART to be used by the debugger.  See mri.h for
#             allowed values.
#             default: MRI_UART_MBED_USB - Use USB based UART on the mbed.
# Example makefile:
#       PROJECT=HelloWorld
#       SRC=.
#       GCC4MBED_DIR=../..
#       LIBS_PREFIX=../agutil/agutil.ar
#       LIBS_SUFFIX=
#
#       include ../../build/gcc4mbed.mk
#      
###############################################################################

# Check for undefined variables.
ifndef PROJECT
$(error makefile must set PROJECT variable.)
endif

ifndef GCC4MBED_DIR
$(error makefile must set GCC4MBED_DIR.)
endif


# Set VERBOSE make variable to 1 to output all tool commands.
VERBOSE?=0
ifeq "$(VERBOSE)" "0"
Q := @
else
Q :=
endif


# Default variables.
SRC ?= .
GCC4MBED_TYPE ?= Release
MRI_BREAK_ON_INIT ?= 1
MRI_UART ?= MRI_UART_MBED_USB


# Configure MRI variables based on GCC4MBED_TYPE build type variable.
ifeq "$(GCC4MBED_TYPE)" "Release"
OPTIMIZATION ?= 2
MRI_ENABLE := 0
MRI_SEMIHOST_STDIO ?= 0
endif


ifeq "$(GCC4MBED_TYPE)" "Debug"
OPTIMIZATION = 0
MRI_ENABLE := 1
MRI_SEMIHOST_STDIO ?= 1
endif


ifeq "$(GCC4MBED_TYPE)" "Checked"
OPTIMIZATION ?= 2
MRI_ENABLE := 1
MRI_SEMIHOST_STDIO ?= 1
endif

MRI_INIT_PARAMETERS := $(MRI_UART)


# Output Object Directory.
OUTDIR := LPC176x

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
# UNDONE: This will need to work across multiple mbed SDK targets.
INCDIRS  += $(MBED_DIR)/targets/hal/TARGET_NXP/TARGET_LPC176X
INCDIRS  += $(MBED_DIR)/targets/cmsis/TARGET_NXP/TARGET_LPC176X

# DEFINEs to be used when building C/C++ code
# UNDONE: Is device specific
DEFINES += -DTARGET_LPC1768 -DTOOLCHAIN_GCC_ARM -DTOOLCHAIN_GCC -D__CORTEX_M3
DEFINES += -DMRI_ENABLE=$(MRI_ENABLE) -DMRI_INIT_PARAMETERS='"$(MRI_INIT_PARAMETERS)"' 
DEFINES += -DMRI_BREAK_ON_INIT=$(MRI_BREAK_ON_INIT) -DMRI_SEMIHOST_STDIO=$(MRI_SEMIHOST_STDIO)

# Libraries to be linked into final binary
SYS_LIBS  := -lstdc++_s -lsupc++_s -lm -lgcc -lc_s -lgcc -lc_s -lnosys
LIBS      := $(LIBS_PREFIX) 

# Some choices like mbed library and enabling of asserts depend on build type.
ifeq "$(GCC4MBED_TYPE)" "Debug"
MBED_LIBS := $(EXTERNAL_DIR)/mbed/libraries/mbed/Debug/NXP_LPC176X/libmbed.a
else
MBED_LIBS := $(EXTERNAL_DIR)/mbed/libraries/mbed/Release/NXP_LPC176X/libmbed.a
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
$(TARGET_BIN): CPP_FLAGS := $(GPFLAGS) -O$(OPTIMIZATION) -g3 -mcpu=cortex-m3 -mthumb -mthumb-interwork 
$(TARGET_BIN): CPP_FLAGS += -ffunction-sections -fdata-sections  -fno-exceptions -fno-delete-null-pointer-checks
$(TARGET_BIN): CPP_FLAGS += $(patsubst %,-I%,$(INCDIRS))
$(TARGET_BIN): CPP_FLAGS += $(DEFINES)
$(TARGET_BIN): CPP_FLAGS += $(DEPFLAGS)
$(TARGET_BIN): CPP_FLAGS += -Wall -Wextra -Wno-unused-parameter

$(TARGET_BIN): C_FLAGS := $(GCFLAGS) $(CPP_FLAGS)

$(TARGET_BIN): ASM_FLAGS     := -g3 -mcpu=cortex-m3 -mthumb
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
$(TARGET_BIN): LD_FLAGS := -mcpu=cortex-m3 -mthumb -O$(OPTIMIZATION) -specs=$(GCC4MBED_DIR)/build/startfile.spec
$(TARGET_BIN): LD_FLAGS  += -Wl,-Map=$(OUTDIR)/$(PROJECT).map,--cref,--gc-sections,--wrap=_isatty,--wrap=malloc,--wrap=realloc,--wrap=free$(MRI_WRAPS)
ifneq "$(NO_FLOAT_SCANF)" "1"
$(TARGET_BIN): LD_FLAGS += -u _scanf_float
endif
ifneq "$(NO_FLOAT_PRINTF)" "1"
$(TARGET_BIN): LD_FLAGS += -u _printf_float
endif


#  Compiler/Assembler/Linker Paths
GCC     := arm-none-eabi-gcc
GPP     := arm-none-eabi-g++
AS      := arm-none-eabi-as
AR      :=arm-none-eabi-ar
LD      := arm-none-eabi-g++
OBJCOPY := arm-none-eabi-objcopy
OBJDUMP := arm-none-eabi-objdump
SIZE    := arm-none-eabi-size

# Some tools are different on Windows in comparison to Unix.
ifeq "$(OS)" "Windows_NT"
REMOVE = del
SHELL=cmd.exe
REMOVE_DIR = rd /s /q
MKDIR = mkdir
QUIET=>nul 2>nul & exit 0
BLANK_LINE=echo -
else
REMOVE = rm
REMOVE_DIR = rm -r -f
MKDIR = mkdir -p
QUIET=> /dev/null 2>&1 ; exit 0
BLANK_LINE=echo
endif

# Create macro which will convert / to \ on Windows.
ifeq "$(OS)" "Windows_NT"
define convert-slash
$(subst /,\,$1)
endef
else
define convert-slash
$1
endef
endif


#########################################################################
.PHONY: all clean clean-all deploy size

all:: $(TARGET_BIN) $(OUTDIR)/$(PROJECT).hex $(OUTDIR)/$(PROJECT).disasm size

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

size: $(OUTDIR)/$(PROJECT).elf
	$(Q) $(SIZE) $<
	@$(BLANK_LINE)

clean: CLEAN_TARGET := $(OUTDIR)
clean: PROJECT      := $(PROJECT)
clean:
	@echo Cleaning $(PROJECT)/$(CLEAN_TARGET)
	$(Q) $(REMOVE_DIR) $(CLEAN_TARGET) $(QUIET)

clean-all: NXP_LPC176X_clean clean

-include $(DEPFILES)

ifdef LPC_DEPLOY
deploy: $(OUTDIR)/$(PROJECT).elf
	@echo Deploying to target.
	$(Q) $(subst PROJECT,$(OUTDIR)/$(PROJECT),$(LPC_DEPLOY))
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


# Include makefiles that enable the building of mbed SDK dependencies.
include $(GCC4MBED_DIR)/build/mbed-lpc1768.mk
