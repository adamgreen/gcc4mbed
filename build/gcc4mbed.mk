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
#   DEVICES: Used to specify a space delimited list of target device(s) that
#             this application should be built for.  Allowed values include:
#              LPC1768
#              LPC11U24
#              KL25Z
#              NRF51822
#              default: LPC1768
#   SRC: The root directory for the sources of your project.  Defaults to '.'.
#   EXCLUDE: Patterns of file to exclude from the root directory of sources 
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
#               net/eth
#               rtos
#               fs
#               rpc
#               dsp
#               USBDevice
#               USBHost
#   DEFINES: Project specific #defines to be set when compiling main
#            application.  Each macro should start with "-D" as required by
#            GCC.
#   INCDIRS: Space delimited list of extra directories to use for #include
#            searches.
#   LIBS_PREFIX: List of library/object files to prepend to mbed libs.
#   LIBS_SUFFIX: List of library/object files to append to mbed libs.
#   GPFLAGS: Additional compiler flags used when building C++ sources.
#   GCFLAGS: Additional compiler flags used when building C sources.
#   GAFLAGS: Additional assembler flags used when building assembly language
#             sources.
#   OPTIMIZATION: Optional variable that can be set to s, g, 0, 1, 2, or 3 for
#                 overriding the compiler's optimization level.  It defaults
#                 to 2 for Checked and Release buillds and is forced to be g
#                 for Debug builds.
#   NEWLIB_NANO: When set to 1, use the smaller newlib-nano C libraries and
#                use the standard newlib libraries otherwise.  It defaults
#                to a value of 1 so that smaller builds are produced.
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
NEWLIB_NANO       ?= 1


# Configure MRI variables based on GCC4MBED_TYPE build type variable.
ifeq "$(GCC4MBED_TYPE)" "Release"
OPTIMIZATION ?= 2
MRI_ENABLE := 0
MRI_SEMIHOST_STDIO := 0
endif


ifeq "$(GCC4MBED_TYPE)" "Debug"
OPTIMIZATION ?= 0
MRI_ENABLE ?= 1
MRI_SEMIHOST_STDIO ?= $(MRI_ENABLE)
endif


ifeq "$(GCC4MBED_TYPE)" "Checked"
OPTIMIZATION ?= 2
MRI_ENABLE := 1
MRI_SEMIHOST_STDIO ?= $(MRI_ENABLE)
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


# Used on linker command line to pull all object files from mbed.a.  Unused modules will be garbage collected away.
WHOLE_ARCHIVE   := -Wl,-whole-archive
NOWHOLE_ARCHIVE := -Wl,-no-whole-archive
all_objs_from_mbed = $(patsubst %mbed.a,$(WHOLE_ARCHIVE) %mbed.a $(NOWHOLE_ARCHIVE),$1)


# Add in library dependencies.
MBED_LIBS := $(patsubst net/eth,net/lwip net/eth rtos,$(MBED_LIBS))
MBED_LIBS := $(patsubst USBHost,USBHost fs rtos,$(MBED_LIBS))


# Directories where mbed source files are found.
MBED_LIB_SRC_ROOT    := $(GCC4MBED_DIR)/external/mbed/libraries
MBED_SRC_ROOT        := $(MBED_LIB_SRC_ROOT)/mbed


# Root directories for official mbed library output.
LIB_RELEASE_DIR := $(GCC4MBED_DIR)/external/mbed/Release
LIB_DEBUG_DIR   := $(GCC4MBED_DIR)/external/mbed/Debug


# Toolchain sub-directories to be built with GCC.
TOOLCHAINS := TOOLCHAIN_GCC TOOLCHAIN_GCC_ARM
TOOLCHAIN_DEFINES := $(patsubst %,-D%,$(TOOLCHAINS))


# Macros for selecting sources/objects to be built for a project.
src_ext     := c cpp S
ifneq "$(OS)" "Windows_NT"
src_ext     +=  s
recurse_dir = $(patsubst %/,%,$(sort $1 $(shell find $1 -type d)))
else
win32_find = $(patsubst $(shell cmd /v:on /c "pushd $1 && echo !CD!&& popd")%,$1%,$(shell dir /s /ad /b $1))
recurse_dir = $(patsubst %/,%,$(sort $1 $(subst \,/,$(call win32_find,$(call convert-slash,$1)))))
endif
find_srcs   = $(subst //,/,$(foreach i,$(src_ext),$(foreach j,$1,$(wildcard $j/*.$i))))
srcs2objs   = $(patsubst $2/%,$3/%,$(addsuffix .o,$(basename $(call find_srcs,$1))))
all_targets = $(sort $(filter TARGET_%,$(notdir $1)))
unsupported_targets = $(filter-out $2,$(call all_targets,$1))
unsupported_target_dirs = $(filter $(addprefix %/,$(call unsupported_targets,$1,$2)),$1)
filter_targets = $(patsubst %/,%,$(filter-out $(addsuffix /%,$(call unsupported_target_dirs,$1,$2)),$(addsuffix /,$1)))
all_toolchains = $(sort $(filter TOOLCHAIN_%,$(notdir $1)))
unsupported_toolchains = $(filter-out $2,$(call all_toolchains,$1))
unsupported_toolchain_dirs = $(filter $(addprefix %/,$(call unsupported_toolchains,$1,$2)),$1)
filter_toolchains = $(patsubst %/,%,$(filter-out $(addsuffix /%,$(call unsupported_toolchain_dirs,$1,$(TOOLCHAINS))),$(addsuffix /,$1)))
filter_dirs = $(call filter_toolchains,$(call filter_targets,$1,$2))


# Utility macros to help build mbed SDK libraries.
define build_lib #,libname,source_dirs,include_dirs
    # Release and Debug target libraries for C and C++ portions of library.
    RELEASE_LIB  := $(RELEASE_DIR)/$1.a
    DEBUG_LIB    := $(DEBUG_DIR)/$1.a

    # Convert list of source files to corresponding list of object files to be generated.
    OBJECTS         := $(call srcs2objs,$2,$(MBED_LIB_SRC_ROOT),__Output__)
    DEBUG_OBJECTS   := $$(patsubst __Output__%,$(DEBUG_DIR)%,$$(OBJECTS))
    RELEASE_OBJECTS := $$(patsubst __Output__%,$(RELEASE_DIR)%,$$(OBJECTS))

    # List of the header dependency files, one per object file.
    DEPFILES += $$(patsubst %.o,%.d,$$(DEBUG_OBJECTS))
    DEPFILES += $$(patsubst %.o,%.d,$$(RELEASE_OBJECTS))

    # Append to main project's include path.
    MBED_INCLUDES += $3

    # Customize C/C++/ASM flags for Debug and Release builds.
    $$(DEBUG_LIB): C_FLAGS   := $(C_FLAGS) -O$(DEBUG_OPTIMIZATION)
    $$(DEBUG_LIB): CPP_FLAGS := $(CPP_FLAGS) -O$(DEBUG_OPTIMIZATION)
    $$(RELEASE_LIB): C_FLAGS   := $(C_FLAGS) -O$(RELEASE_OPTIMIZATION) -DNDEBUG
    $$(RELEASE_LIB): CPP_FLAGS := $(CPP_FLAGS) -O$(RELEASE_OPTIMIZATION) -DNDEBUG
    $$(RELEASE_LIB): ASM_FLAGS := $(ASM_FLAGS)
    $$(DEBUG_LIB):   ASM_FLAGS := $(ASM_FLAGS)

    #########################################################################
    # High level rules for building Debug and Release versions of library.
    #########################################################################
    $$(RELEASE_LIB): $$(RELEASE_OBJECTS)
		@echo Linking release library $@
		$(Q) $(MKDIR) $$(call convert-slash,$$(dir $$@)) $(QUIET)
		$(Q) $(AR) -rc $$@ $$+

    $$(DEBUG_LIB): $$(DEBUG_OBJECTS)
		@echo Linking debug library $@
		$(Q) $(MKDIR) $$(call convert-slash,$$(dir $$@)) $(QUIET)
		$(Q) $(AR) -rc $$@ $$+

endef


# Rules for building all of the desired device targets
all: $(DEVICES)
clean: $(addsuffix -clean,$(DEVICES))
clean-all: clean
	@echo Cleaning $(LIB_RELEASE_DIR)
	$(Q) $(REMOVE_DIR) $(call convert-slash,$(LIB_RELEASE_DIR)) $(QUIET)
	@echo Cleaning $(LIB_DEBUG_DIR)
	$(Q) $(REMOVE_DIR) $(call convert-slash,$(LIB_DEBUG_DIR)) $(QUIET)

deploy: LPC1768-deploy


# Determine supported devices by looking at *-device.mk makefiles.
ALL_DEVICE_MAKEFILES := $(wildcard $(GCC4MBED_DIR)/build/*-device.mk)
ALL_DEVICES          := $(patsubst $(GCC4MBED_DIR)/build/%-device.mk,%,$(ALL_DEVICE_MAKEFILES))


# Include makefiles that know how to build each of the supported device types.
include $(ALL_DEVICE_MAKEFILES)
