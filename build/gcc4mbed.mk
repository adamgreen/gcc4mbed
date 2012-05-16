#Copyright (C) 2011 by Sagar G V
#
#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in
#all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#THE SOFTWARE.
#
# Updates: 
#    Arthur Wolf & Adam Green in 2011 - Updated to work with mbed.
###############################################################################
# USAGE:
# Variables that must be defined in including makefile.
#   PROJECT: Name to be given to the output binary for this project.
#   SRC: The root directory for the sources of your project.
#   GCC4MED_DIR: The root directory for where the gcc4mbed sources are located
#                in your project.  This should point to the parent directory
#                of the build directory which contains this gcc4mbed.mk file.
#   LIBS_PREFIX: List of library/object files to prepend to mbed.ar capi.ar libs.
#   LIBS_SUFFIX: List of library/object files to append to mbed.ar capi.ar libs.
#   GCC4MBED_DELAYED_STDIO_INIT: Set to non-zero value to have intialization of
#                                stdin/stdout/stderr delayed which will
#                                shrink the size of the resulting binary if
#                                APIs like printf(), scanf(), etc. aren't used.
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

# Default project source to be located in current directory.
ifndef SRC
SRC=.
endif

# Default the init of stdio/stdout/stderr to occur before global constructors.
ifndef GCC4MBED_DELAYED_STDIO_INIT
GCC4MBED_DELAYED_STDIO_INIT=0
endif

# List of sources to be compiled/assembled
CSRCS = $(wildcard $(SRC)/*.c $(SRC)/*/*.c $(SRC)/*/*/*.c $(SRC)/*/*/*/*.c $(SRC)/*/*/*/*/*.c)
ASRCS =  $(wildcard $(SRC)/*.S $(SRC)/*/*.S $(SRC)/*/*/*.S $(SRC)/*/*/*/*.S $(SRC)/*/*/*/*/*.S)
CPPSRCS = $(wildcard $(SRC)/*.cpp $(SRC)/*/*.cpp $(SRC)/*/*/*.cpp $(SRC)/*/*/*/*.cpp $(SRC)/*/*/*/*/*.cpp)

# List of the objects files to be compiled/assembled
OBJECTS= $(CSRCS:.c=.o) $(ASRCS:.S=.o) $(CPPSRCS:.cpp=.o)

# Linker script to be used.  Indicates what code should be placed where in memory.
LSCRIPT=$(GCC4MBED_DIR)/build/mbed.ld

# Location of external library and header dependencies.
EXTERNAL_DIR = $(GCC4MBED_DIR)/external

# Include path which points to external library headers and to subdirectories of this project which contain headers.
SUBDIRS = $(wildcard $(SRC)/* $(SRC)/*/* $(SRC)/*/*/* $(SRC)/*/*/*/* $(SRC)/*/*/*/*/*)
PROJINCS = $(sort $(dir $(SUBDIRS)))
INCDIRS += $(PROJINCS) $(EXTERNAL_DIR)/mbed $(EXTERNAL_DIR)/mbed/LPC1768/GCC_ARM $(EXTERNAL_DIR)/FATFileSystem

# DEFINEs to be used when building C/C++ code
DEFINES = -DTARGET_LPC1768 -DGCC4MBED_DELAYED_STDIO_INIT=$(GCC4MBED_DELAYED_STDIO_INIT)

# Libraries to be linked into final binary
MBED_LIBS = $(EXTERNAL_DIR)/mbed/LPC1768/GCC_ARM/libmbed.a $(EXTERNAL_DIR)/mbed/LPC1768/GCC_ARM/libcapi.a
SYS_LIBS = -lstdc++ -lsupc++ -lm -lc -lnosys -lgcc
LIBS = $(LIBS_PREFIX) 
LIBS += $(EXTERNAL_DIR)/mbed/LPC1768/GCC_ARM/startup_LPC17xx.o
LIBS += $(EXTERNAL_DIR)/mbed/LPC1768/GCC_ARM/cmsis_nvic.o
LIBS += $(EXTERNAL_DIR)/mbed/LPC1768/GCC_ARM/core_cm3.o
LIBS += $(EXTERNAL_DIR)/mbed/LPC1768/GCC_ARM/system_LPC17xx.o
LIBS += $(MBED_LIBS)
LIBS += $(SYS_LIBS)
LIBS += $(MBED_LIBS)
LIBS += $(SYS_LIBS)
LIBS += $(LIBS_SUFFIX)

# Optimization level
OPTIMIZATION = 2

#  Compiler Options
GPFLAGS = -O$(OPTIMIZATION) -g -mcpu=cortex-m3 -mthumb -mthumb-interwork 
GPFLAGS += -ffunction-sections -fdata-sections  -fno-exceptions 
GPFLAGS += -Wall -Wextra -Wno-unused-parameter -Wcast-align -Wpointer-arith -Wredundant-decls -Wcast-qual -Wcast-align
GPFLAGS += $(patsubst %,-I%,$(INCDIRS))
GPFLAGS += $(DEFINES)

LDFLAGS = -mcpu=cortex-m3 -mthumb -O$(OPTIMIZATION) -Wl,-Map=$(PROJECT).map,--cref,--gc-sections -T$(LSCRIPT)

ASFLAGS = $(LISTING) -mcpu=cortex-m3 -mthumb -x assembler-with-cpp
ASFLAGS += $(patsubst %,-I%,$(INCDIRS))

#  Compiler/Assembler/Linker Paths
GPP = arm-none-eabi-g++
AS = arm-none-eabi-gcc
LD = arm-none-eabi-g++
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE = arm-none-eabi-size
REMOVE = rm

# Switch to cs-rm on Windows and make sure that cmd.exe is used as shell.
ifeq "$(MAKE)" "cs-make"
REMOVE = cs-rm
SHELL=cmd.exe
endif

#########################################################################

all:: $(PROJECT).hex $(PROJECT).bin $(PROJECT).disasm

$(PROJECT).bin: $(PROJECT).elf
	$(OBJCOPY) -O binary $(PROJECT).elf $(PROJECT).bin

$(PROJECT).hex: $(PROJECT).elf
	$(OBJCOPY) -R .stack -O ihex $(PROJECT).elf $(PROJECT).hex
	
$(PROJECT).disasm: $(PROJECT).elf
	$(OBJDUMP) -d $(PROJECT).elf >$(PROJECT).disasm
	
$(PROJECT).elf: $(LSCRIPT) $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(PROJECT).elf
	$(SIZE) $(PROJECT).elf

clean:
	$(REMOVE) -f $(OBJECTS)
	$(REMOVE) -f $(PROJECT).hex
	$(REMOVE) -f $(PROJECT).elf
	$(REMOVE) -f $(PROJECT).map
	$(REMOVE) -f $(PROJECT).bin
	$(REMOVE) -f $(PROJECT).disasm

ifdef LPC_DEPLOY
DEPLOY_COMMAND = $(subst PROJECT,$(PROJECT),$(LPC_DEPLOY))
deploy:
	$(DEPLOY_COMMAND)
endif

#########################################################################
#  Default rules to compile .c and .cpp file to .o
#  and assemble .s files to .o

.c.o :
	$(GPP) $(GPFLAGS) -c $< -o $(<:.c=.o)

.cpp.o :
	$(GPP) $(GPFLAGS) -c $< -o $(<:.cpp=.o)

.S.o :
	$(AS) $(ASFLAGS) -c $< -o $(<:.S=.o)

#########################################################################