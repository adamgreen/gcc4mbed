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
# Summary folows but more information can be found at following link:
#  https://github.com/adamgreen/gcc4mbed/blob/master/notes/makefile.creole
#
# Variables that must be defined in including makefile.
#   PROJECT: Name to be given to the output binary for this project.
#   GCC4MBED_DIR: The root directory for where the gcc4mbed sources are located
#                 in your project.  This should point to the parent directory
#                 of the build directory which contains this gcc4mbed.mk file.
#
# Variables that may be optionally set in makefile.
#   DEVCICES: Used to specify a space delimited list of target device(s) that
#             this application should be built for.  Allowed values include:
#              LPC1768
#              LPC11U24
#              KL25Z
#              default: LPC1768
#   SRC: The root directory for the sources of your project.  Defaults to '.'.
#   NO_FLOAT_SCANF: When set to 1, scanf() will not support %f specifier to
#                   input floating point values.  Reduces code size.
#   NO_FLOAT_PRINTF: When set to 1, scanf() will not support %f specifier to
#                    output floating point values.  Reduces code size.
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
#   MBED_LIBS: Specifies which additional official mbed libraries you would
#              like to use with your application.  These include:
#               net/lwip
#               net/eth
#               rtos
#   INCDIRS: Space delimited list of extra directories to use for #include
#            searches.
#   LIBS_PREFIX: List of library/object files to prepend to mbed libs.
#   LIBS_SUFFIX: List of library/object files to append to mbed libs.
#   GPFLAGS: Additional compiler flags used when building C++ sources.
#   GCFLAGS: Additional compiler flags used when building C sources.
#   AS_FLAGS: Additional assembler flags used when building assembly language
#             sources.
#   OPTIMIZATION: Optional variable that can be set to s, 0, 1, 2, or 3 for
#                 overriding the compiler's optimization level.  It defaults
#                 to 2 for Checked and Release buillds and is forced to be 0
#                 for Debug builds.
#   VERBOSE: When set to 1, all build commands will be displayed to console.
#            It defaults to 0 which suppresses the output of the build tool
#            command lines themselves.
#   MRI_ENABLE: Is typically set based on GCC4MBED_TYPE but defaults to 1 for
#               Debug (non-optimized builds).  User can force it to 0 for
#               Debug builds so that the MRI debug monitor isn't used.  Useful
#               for creating builds to use with JTAG debuggers.
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
#
# Example makefile:
#       PROJECT      := HelloWorld
#       SRC          := .
#       GCC4MBED_DIR := ../..
#       INCDIRS      :=
#       LIBS_PREFIX  :=
#       LIBS_SUFFIX  :=
#
#       include $(GCC4MBED_DIR)/build/gcc4mbed.mk
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
SRC               ?= .
GCC4MBED_TYPE     ?= Release
NO_FLOAT_SCANF    ?= 0
NO_FLOAT_PRINTF   ?= 0
MRI_BREAK_ON_INIT ?= 1
MRI_UART          ?= MRI_UART_MBED_USB
DEVICES           ?= LPC1768


# Configure MRI variables based on GCC4MBED_TYPE build type variable.
ifeq "$(GCC4MBED_TYPE)" "Release"
OPTIMIZATION ?= 2
MRI_ENABLE := 0
MRI_SEMIHOST_STDIO ?= 0
endif


ifeq "$(GCC4MBED_TYPE)" "Debug"
OPTIMIZATION ?= 0
MRI_ENABLE ?= 1
MRI_SEMIHOST_STDIO ?= 1
endif


ifeq "$(GCC4MBED_TYPE)" "Checked"
OPTIMIZATION ?= 2
MRI_ENABLE := 1
MRI_SEMIHOST_STDIO ?= 1
endif

MRI_INIT_PARAMETERS := $(MRI_UART)


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


# Make sure that the mbed library always gets linked in.
MBED_LIBS += mbed


# Add in library dependencies.
MBED_LIBS := $(patsubst net/eth,net/lwip net/eth rtos,$(MBED_LIBS))
MBED_LIBS := $(patsubst USBHost,USBHost fs rtos,$(MBED_LIBS))


# All supported devices that clean-all target should clean.
ALL_DEVICES := LPC1768 LPC11U24 KL25Z


# Rules for building all of the desired device targets
all: $(DEVICES)
clean: $(addsuffix _clean,$(DEVICES))
clean-all: $(addsuffix _MBED_clean,$(ALL_DEVICES)) clean
deploy: LPC1768_deploy


# Include makefiles that know how to build each of the supported device types.
include $(GCC4MBED_DIR)/build/device-lpc1768.mk
include $(GCC4MBED_DIR)/build/device-lpc11u24.mk
include $(GCC4MBED_DIR)/build/device-kl25z.mk
