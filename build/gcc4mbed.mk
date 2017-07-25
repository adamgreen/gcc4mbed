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
# Variables that may optionally be set in makefile.
#   DEVICES: Used to specify a space delimited list of target device(s) that
#             this application should be built for.  Allowed values include:
#              LPC1768
#              LPC11U24
#              KL25Z
#              NRF51822
#              default: LPC1768
#   SRC: The root directory for the sources of your project.  Defaults to '.'.
#   GCC4MBED_TYPE: Type of build to produce.  Allowed values are:
#                  Debug - Build for debugging.  Disables optimizations,
#                          enables asserts and other debug code, doesn't
#                          allow the RTOS to put CPU in sleep mode.  Best 
#                          debugging experience.
#                  Release - Build for release. Optimizes for space, sets 
#                            NDEBUG macro to disable asserts and other debug
#                            code, and allows the RTOS to put CPU in sleep 
#                            mode.
#                  Develop - The same as Release except that it doesn't set
#                            the NDEBUG macro and doesn't allow the RTOS to put
#                            CPU in sleep mode.
#                  default: Release
#   MBED_OS_ENABLE:  When set to 1, link with the full mbed-os library which
#                    includes RTOS support (mbed-os 5). Setting to 0 will link
#                    with the core mbed SDK library with no RTOS support 
#                    (mbed 2). It defaults to 1 to support the more full
#                    featured mbed-os 5.
#   NEWLIB_NANO: When set to 1, use the smaller newlib-nano C libraries and
#                use the standard newlib libraries otherwise.  It defaults
#                to a value of 1 so that smaller builds are produced.
#   NO_FLOAT_SCANF: When set to 1, scanf() will not support %f specifier to
#                   input floating point values.  Reduces code size. Defaults
#                   to 0.
#   NO_FLOAT_PRINTF: When set to 1, scanf() will not support %f specifier to
#                    output floating point values.  Reduces code size. Defaults
#                    to 0.
#   DEFINES: Project specific #defines to be set when compiling main
#            application.  Each macro should start with "-D" as required by
#            GCC.
#   USER_LIBS: A space delimited list of folders containing user libraries to
#              be compiled and linked into the application. Normally it will
#              recurse into all subfolders within a user library to generate
#              the include path for the library. If you preceed a folder in the
#              list with a '!' character then it will only add the root library
#              folder to the include path.
#   MBED_CONFIG_H: Allows the user to specify the pathname of a header file 
#                  containing custom mbed configuration parameters. It defaults
#                  to using src/mbed_config.h
#   INCDIRS: Space delimited list of extra directories to use for #include
#            searches.
#   *_LSCRIPT: The linker script to be used for a particular device can be
#              overridden by setting the appropriate *_LSCRIPT variable.  For
#              example setting "LPC1768_LSCRIPT := lpc1768custom.ld" will cause
#              the linker to use "lpc1768custom.ld" instead of the one provided
#              by the mbed SDK when building for the LPC1768 target.
#   LIBS_PREFIX: List of library/object files to prepend to mbed libs.
#   LIBS_SUFFIX: List of library/object files to append to mbed libs.
#   GPFLAGS: Additional compiler flags used when building C++ sources.
#   GCFLAGS: Additional compiler flags used when building C sources.
#   GAFLAGS: Additional assembler flags used when building assembly language
#             sources.
#   OPTIMIZATION: Optional variable that can be set to s, g, 0, 1, 2, or 3 for
#                 overriding the compiler's optimization level.  It defaults
#                 to s (size optimization) for Develop and Release builds and
#                 to 0 (optimizations disabled) for Debug builds.
#                 This optimization is only used for the application's
#                 code itself and the libraries are always built with the
#                 default for the specified GCC4MBED_TYPE.
#   MRI_ENABLE: Set to 1 to enable the MRI debug monitor to be linked into
#               the executable for devices that MRI supports. Defaults to 0.
#   MRI_BREAK_ON_INIT: Should the program halt before calling into main(),
#                      allowing the developer time to set breakpoints in main()
#                      or in code run from within global constructors.
#                      default: 1 - break on init.
#   MRI_SEMIHOST_STDIO: Set to non-zero value to allow debug monitor to use
#                       semi-host calls to redirect stdin/stdout/stderr to the
#                       gdb console.
#                       default: 1 when MRI_ENABLE is 1 and 0 otherwise.
#   MRI_UART: Select the UART to be used by the debugger.  See mri.h for
#             allowed values.
#             default: MRI_UART_MBED_USB - Use USB based UART on the mbed.
#   GCC4MBED_TOOLPATH: Optional path to where the GNU tools are located. It
#                      defaults to $(GCC4MBED_DIR)/gcc-arm-none-eabi/bin
#   EXCLUDE: Patterns of file to exclude from the root directory of sources 
#   VERBOSE: When set to 1, all build commands will be displayed to console.
#            It defaults to 0 which suppresses the output of the build tool
#            command lines themselves.
#
# Example makefile:
#   PROJECT         := HelloWorld
#   DEVICES         := LPC1768
#
#   GCC4MBED_DIR    := ../..
#
#   NO_FLOAT_SCANF  := 1
#   NO_FLOAT_PRINTF := 1
#
#   include $(GCC4MBED_DIR)/build/gcc4mbed.mk
#      
###############################################################################

# Check for undefined variables.
ifndef PROJECT
$(error makefile must set PROJECT variable.)
endif

ifndef GCC4MBED_DIR
$(error makefile must set GCC4MBED_DIR.)
endif

# Remove trailing slash in directory name since makefile doesn't expect it.
GCC4MBED_DIR := $(patsubst %/,%,$(GCC4MBED_DIR))

# Set VERBOSE make variable to 1 to output all tool commands.
VERBOSE?=0
ifeq "$(VERBOSE)" "0"
Q := @
else
Q :=
endif


# Default variables.
SRC                ?= .
GCC4MBED_TYPE      ?= Release
NO_FLOAT_SCANF     ?= 0
NO_FLOAT_PRINTF    ?= 0
MRI_ENABLE         ?= 0
MRI_SEMIHOST_STDIO ?= $(MRI_ENABLE)
MRI_BREAK_ON_INIT  ?= 1
MRI_UART           ?= MRI_UART_MBED_USB
DEVICES            ?= LPC1768
NEWLIB_NANO        ?= 1
MBED_CONFIG_H      ?= $(GCC4MBED_DIR)/src/mbed_config.h
MBED_OS_ENABLE     ?= 1


# Optimization levels to be used for Debug and Release versions of libraries.
DEBUG_OPTIMIZATION   := 0
DEVELOP_OPTIMIZATION := s
RELEASE_OPTIMIZATION := s


# Configure variables based on GCC4MBED_TYPE setting.
# BUILD_TYPE_TARGET is set to TARGET_DEBUG for Debug builds and TARGET_RELEASE for all other builds types to be
# consistent with mbed build tools.
VALID_TYPE := 0
ifeq "$(GCC4MBED_TYPE)" "Debug"
OPTIMIZATION ?= $(DEBUG_OPTIMIZATION)
BUILD_TYPE_TARGET := TARGET_DEBUG
VALID_TYPE := 1
endif

ifeq "$(GCC4MBED_TYPE)" "Develop"
OPTIMIZATION ?= $(DEVELOP_OPTIMIZATION)
BUILD_TYPE_TARGET := TARGET_RELEASE
VALID_TYPE := 1
endif

ifeq "$(GCC4MBED_TYPE)" "Release"
OPTIMIZATION ?= $(RELEASE_OPTIMIZATION)
BUILD_TYPE_TARGET := TARGET_RELEASE
VALID_TYPE := 1
endif

ifeq "$(VALID_TYPE)" "0"
$(error makefile must set GCC4MBED_TYPE to Debug, Develop, or Release.)
endif


MRI_INIT_PARAMETERS := $(MRI_UART)


# Some tools are different on Windows in comparison to Unix.
ifeq "$(OS)" "Windows_NT"
REMOVE = del
SHELL=cmd.exe
REMOVE_DIR = rd /s /q
MKDIR = mkdir
QUIET=>nul 2>nul & exit 0
else
REMOVE = rm
REMOVE_DIR = rm -r -f
MKDIR = mkdir -p
QUIET=> /dev/null 2>&1 ; exit 0
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


# Compiler/Assembler/Linker Paths
#   Default to using tools installed in GCC4MBED folder if user hasn't explicitly specified their location.
GCC4MBED_TOOLPATH ?= $(GCC4MBED_DIR)/gcc-arm-none-eabi/bin
GCC     := $(call convert-slash,$(GCC4MBED_TOOLPATH)/arm-none-eabi-gcc)
GPP     := $(call convert-slash,$(GCC4MBED_TOOLPATH)/arm-none-eabi-g++)
AS      := $(call convert-slash,$(GCC4MBED_TOOLPATH)/arm-none-eabi-as)
AR      := $(call convert-slash,$(GCC4MBED_TOOLPATH)/arm-none-eabi-ar)
LD      := $(call convert-slash,$(GCC4MBED_TOOLPATH)/arm-none-eabi-g++)
OBJCOPY := $(call convert-slash,$(GCC4MBED_TOOLPATH)/arm-none-eabi-objcopy)
OBJDUMP := $(call convert-slash,$(GCC4MBED_TOOLPATH)/arm-none-eabi-objdump)
SIZE    := $(call convert-slash,$(GCC4MBED_TOOLPATH)/arm-none-eabi-size)


# Pick the mbed library to use based on MBED_OS_ENABLE setting.
# 0 links in mbed.a for single threaded projects.
# 1 links in mbedos.a for multi-threaded projects.
ifeq "$(MBED_OS_ENABLE)" "0"
MBED_LIB_NAME := mbed
MBED_DEFINES  := 
else
MBED_LIB_NAME := mbedos
MBED_DEFINES  := -DMBED_CONF_RTOS_PRESENT=1 -DMBED_CONF_NSAPI_PRESENT=1
endif
MBED_LIBS     += $(MBED_LIB_NAME)


# Only release builds disable asserts and enable RTOS sleeping via setting of NDEBUG macro.
ifeq "$(GCC4MBED_TYPE)" "Release"
MBED_DEFINES += -DNDEBUG
endif

# mbed build tools always define these macros so we do too.
MBED_DEFINES += -D__MBED__=1 -DTARGET_LIKE_MBED


# Have linker pull all object files from mbed library. 
# Unused modules will still be garbage collected away in final pass.
WHOLE_ARCHIVE   := -Wl,-whole-archive
NOWHOLE_ARCHIVE := -Wl,-no-whole-archive
all_objs_from_mbed = $(patsubst %$(MBED_LIB_NAME).a,$(WHOLE_ARCHIVE) %$(MBED_LIB_NAME).a $(NOWHOLE_ARCHIVE),$1)


# Directories where mbed source files are found.
MBED_SRC_ROOT        := $(GCC4MBED_DIR)/external/mbed-os


# Root directories for official mbed library output.
MBED_DEBUG_DIR   := $(MBED_SRC_ROOT)/Debug/$(MBED_LIB_NAME)
MBED_DEVELOP_DIR := $(MBED_SRC_ROOT)/Develop/$(MBED_LIB_NAME)
MBED_RELEASE_DIR := $(MBED_SRC_ROOT)/Release/$(MBED_LIB_NAME)


# Toolchain sub-directories to be built with GCC.
TOOLCHAINS := TOOLCHAIN_GCC TOOLCHAIN_GCC_ARM
TOOLCHAIN_DEFINES := $(patsubst %,-D%,$(TOOLCHAINS))


# Compiler flags used to enable creation of header dependency files.
DEP_FLAGS := -MMD -MP


# Macros for selecting sources/objects to be built for a project.
src_ext     := c cpp S
ifneq "$(OS)" "Windows_NT"
src_ext     +=  s
recurse_dir = $(patsubst %/,%,$(sort $1 $(shell find $1 -type d -and -not -path "*/.git*")))
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
all_features = $(sort $(filter FEATURE_%,$(notdir $1)))
unsupported_features = $(filter-out $2,$(call all_features,$1))
unsupported_feature_dirs = $(filter $(addprefix %/,$(call unsupported_features,$1,$2)),$1)
filter_features = $(patsubst %/,%,$(filter-out $(addsuffix /%,$(call unsupported_feature_dirs,$1,$2)),$(addsuffix /,$1)))
unsupported_test_dirs = $(filter %/TESTS,$1)
filter_out_tests = $(patsubst %/,%,$(filter-out $(addsuffix /%,$(call unsupported_test_dirs,$1)),$(addsuffix /,$1)))
filter_dirs = $(call filter_toolchains,$(call filter_targets,$(call filter_features,$(call filter_out_tests,$1),$3),$2))
remove_ignored_dirs = $(patsubst %/,%,$(filter-out $2,$(addsuffix /,$1)))
find_target_linkscript = $(lastword $(sort $(foreach i,$1,$(wildcard $i/*.ld))))

# Utility macros to help build mbed SDK libraries.
define build_mbed_lib #,libname,source_dirs,include_dirs
    # Release and Debug target libraries for C and C++ portions of library.
    DEBUG_LIB    := $(DEBUG_DIR)/$1.a
    DEVELOP_LIB  := $(DEVELOP_DIR)/$1.a
    RELEASE_LIB  := $(RELEASE_DIR)/$1.a

    # Convert list of source files to corresponding list of object files to be generated.
    OBJECTS         := $(call srcs2objs,$2,$(MBED_SRC_ROOT),__Output__)
    DEBUG_OBJECTS   := $$(patsubst __Output__%,$(DEBUG_DIR)%,$$(OBJECTS))
    DEVELOP_OBJECTS := $$(patsubst __Output__%,$(DEVELOP_DIR)%,$$(OBJECTS))
    RELEASE_OBJECTS := $$(patsubst __Output__%,$(RELEASE_DIR)%,$$(OBJECTS))

    # List of the header dependency files, one per object file.
    DEPFILES += $$(patsubst %.o,%.d,$$(DEBUG_OBJECTS))
    DEPFILES += $$(patsubst %.o,%.d,$$(DEVELOP_OBJECTS))
    DEPFILES += $$(patsubst %.o,%.d,$$(RELEASE_OBJECTS))

    # Append to main project's include path.
    MBED_INCLUDES += $3

    # Customize C/C++/ASM flags for Debug and Release builds.
    $$(DEBUG_LIB): C_FLAGS     := -O$(DEBUG_OPTIMIZATION) $(C_FLAGS)
    $$(DEBUG_LIB): CPP_FLAGS   := -O$(DEBUG_OPTIMIZATION) $(CPP_FLAGS)
    $$(DEBUG_LIB): ASM_FLAGS   := $(ASM_FLAGS)
    $$(DEVELOP_LIB): C_FLAGS   := -O$(DEVELOP_OPTIMIZATION) $(C_FLAGS)
    $$(DEVELOP_LIB): CPP_FLAGS := -O$(DEVELOP_OPTIMIZATION) $(CPP_FLAGS)
    $$(DEVELOP_LIB): ASM_FLAGS := $(ASM_FLAGS)
    $$(RELEASE_LIB): C_FLAGS   := -O$(RELEASE_OPTIMIZATION) $(C_FLAGS)
    $$(RELEASE_LIB): CPP_FLAGS := -O$(RELEASE_OPTIMIZATION) $(CPP_FLAGS)
    $$(RELEASE_LIB): ASM_FLAGS := $(ASM_FLAGS)

    #########################################################################
    # High level rules for building Debug and Release versions of library.
    #########################################################################
    $$(DEBUG_LIB): $$(DEBUG_OBJECTS)
		@echo Linking debug library $$@
		$(Q) $(MKDIR) $$(call convert-slash,$$(dir $$@)) $(QUIET)
		$(Q) $(AR) -rc $$@ $$+

    $$(DEVELOP_LIB): $$(DEVELOP_OBJECTS)
		@echo Linking develop library $$@
		$(Q) $(MKDIR) $$(call convert-slash,$$(dir $$@)) $(QUIET)
		$(Q) $(AR) -rc $$@ $$+

    $$(RELEASE_LIB): $$(RELEASE_OBJECTS)
		@echo Linking release library $$@
		$(Q) $(MKDIR) $$(call convert-slash,$$(dir $$@)) $(QUIET)
		$(Q) $(AR) -rc $$@ $$+

endef


# Utility macros to help build user libraries.
define build_user_lib #,lib_dir
    # Make sure that library directory doesn't have trailing slash and remove any '!' prefix.
    LIB_DIR := $(patsubst !%,%,$(patsubst %/,%,$1))

    # Library name is based on the directory name.
    LIB_NAME := $$(notdir $$(LIB_DIR))

    # Release and Debug target library paths.
    LIB_DEBUG_DIR   := $$(LIB_DIR)/Debug/$(MBED_DEVICE)
    DEBUG_LIB       := $$(LIB_DEBUG_DIR)/lib$$(LIB_NAME).a
    LIB_DEVELOP_DIR := $$(LIB_DIR)/Develop/$(MBED_DEVICE)
    DEVELOP_LIB     := $$(LIB_DEVELOP_DIR)/lib$$(LIB_NAME).a
    LIB_RELEASE_DIR := $$(LIB_DIR)/Release/$(MBED_DEVICE)
    RELEASE_LIB     := $$(LIB_RELEASE_DIR)/lib$$(LIB_NAME).a

    # Find all of the library source directories appropriate for this target device.
    LIB_SRC_DIRS := $(call filter_dirs,$(call recurse_dir,$(patsubst !%,%,$1)),$(TARGETS_FOR_DEVICE),$(FEATURES_FOR_DEVICE))

    # Convert list of source files to corresponding list of object files to be generated.
    OBJECTS         := $$(call srcs2objs,$$(LIB_SRC_DIRS),$$(LIB_DIR),__Output__)
    DEBUG_OBJECTS   := $$(patsubst __Output__%,$$(LIB_DEBUG_DIR)%,$$(OBJECTS))
    DEVELOP_OBJECTS := $$(patsubst __Output__%,$$(LIB_DEVELOP_DIR)%,$$(OBJECTS))
    RELEASE_OBJECTS := $$(patsubst __Output__%,$$(LIB_RELEASE_DIR)%,$$(OBJECTS))

    # List of the header dependency files, one per object file.
    DEPFILES += $$(patsubst %.o,%.d,$$(DEBUG_OBJECTS))
    DEPFILES += $$(patsubst %.o,%.d,$$(DEVELOP_OBJECTS))
    DEPFILES += $$(patsubst %.o,%.d,$$(RELEASE_OBJECTS))

    # Append to main project's include path.
    # If the LIB_DIR starts with a '!' character then don't recurse for the include path.
    LIB_INCLUDES += $(if $(filter-out !%,$1),$$(LIB_SRC_DIRS),$$(LIB_DIR))

    # Customize C/C++/ASM flags for Debug and Release builds.
    $$(DEBUG_LIB): C_FLAGS   := $(C_FLAGS) -O$(DEBUG_OPTIMIZATION)
    $$(DEBUG_LIB): CPP_FLAGS := $(CPP_FLAGS) -O$(DEBUG_OPTIMIZATION)
    $$(DEBUG_LIB): ASM_FLAGS := $(ASM_FLAGS)
    $$(DEVELOP_LIB): C_FLAGS   := $(C_FLAGS) -O$(DEVELOP_OPTIMIZATION)
    $$(DEVELOP_LIB): CPP_FLAGS := $(CPP_FLAGS) -O$(DEVELOP_OPTIMIZATION)
    $$(DEVELOP_LIB): ASM_FLAGS := $(ASM_FLAGS)
    $$(RELEASE_LIB): C_FLAGS   := $(C_FLAGS) -O$(RELEASE_OPTIMIZATION)
    $$(RELEASE_LIB): CPP_FLAGS := $(CPP_FLAGS) -O$(RELEASE_OPTIMIZATION)
    $$(RELEASE_LIB): ASM_FLAGS := $(ASM_FLAGS)

    #########################################################################
    # High level rules for building Debug and Release versions of library.
    #########################################################################
    $$(DEBUG_LIB): $$(DEBUG_OBJECTS)
		@echo Linking debug library $$@
		$(Q) $(MKDIR) $$(call convert-slash,$$(dir $$@)) $(QUIET)
		$(Q) $(AR) -rc $$@ $$+

    $$(DEVELOP_LIB): $$(DEVELOP_OBJECTS)
		@echo Linking develop library $$@
		$(Q) $(MKDIR) $$(call convert-slash,$$(dir $$@)) $(QUIET)
		$(Q) $(AR) -rc $$@ $$+

    $$(RELEASE_LIB): $$(RELEASE_OBJECTS)
		@echo Linking release library $$@
		$(Q) $(MKDIR) $$(call convert-slash,$$(dir $$@)) $(QUIET)
		$(Q) $(AR) -rc $$@ $$+

    #########################################################################
    #  Default rules to compile c/c++/assembly language sources to objects.
    #########################################################################
    $$(LIB_DEBUG_DIR)/%.o : $$(LIB_DIR)/%.c
		@echo Compiling $$<
		$(Q) $(MKDIR) $$(call convert-slash,$$(dir $$@)) $(QUIET)
		$(Q) $(GCC) $$(C_FLAGS) $$(LIB_INCLUDES) $$(MBED_INCLUDES) -I$(GCC4MBED_DIR)/mri -c $$< -o $$@

    $$(LIB_DEBUG_DIR)/%.o : $$(LIB_DIR)/%.cpp
		@echo Compiling $$<
		$(Q) $(MKDIR) $$(call convert-slash,$$(dir $$@)) $(QUIET)
		$(Q) $(GPP) $$(CPP_FLAGS) $$(LIB_INCLUDES) $$(MBED_INCLUDES) -I$(GCC4MBED_DIR)/mri -c $$< -o $$@

    $$(LIB_DEBUG_DIR)/%.o : $$(LIB_DIR)/%.s
		@echo Assembling $$<
		$(Q) $(MKDIR) $$(call convert-slash,$$(dir $$@)) $(QUIET)
		$(Q) $(GCC) $$(ASM_FLAGS) $$(LIB_INCLUDES) $$(MBED_INCLUDES) -I$(GCC4MBED_DIR)/mri -c $$< -o $$@

    $$(LIB_DEBUG_DIR)/%.o : $$(LIB_DIR)/%.S
		@echo Assembling $$<
		$(Q) $(MKDIR) $$(call convert-slash,$$(dir $$@)) $(QUIET)
		$(Q) $(GCC) $$(ASM_FLAGS) $$(LIB_INCLUDES) $$(MBED_INCLUDES) -I$(GCC4MBED_DIR)/mri -c $$< -o $$@

    $$(LIB_DEVELOP_DIR)/%.o : $$(LIB_DIR)/%.c
		@echo Compiling $$<
		$(Q) $(MKDIR) $$(call convert-slash,$$(dir $$@)) $(QUIET)
		$(Q) $(GCC) $$(C_FLAGS) $$(LIB_INCLUDES) $$(MBED_INCLUDES) -I$(GCC4MBED_DIR)/mri -c $$< -o $$@

    $$(LIB_DEVELOP_DIR)/%.o : $$(LIB_DIR)/%.cpp
		@echo Compiling $$<
		$(Q) $(MKDIR) $$(call convert-slash,$$(dir $$@)) $(QUIET)
		$(Q) $(GPP) $$(CPP_FLAGS) $$(LIB_INCLUDES) $$(MBED_INCLUDES) -I$(GCC4MBED_DIR)/mri -c $$< -o $$@

    $$(LIB_DEVELOP_DIR)/%.o : $$(LIB_DIR)/%.s
		@echo Assembling $$<
		$(Q) $(MKDIR) $$(call convert-slash,$$(dir $$@)) $(QUIET)
		$(Q) $(GCC) $$(ASM_FLAGS) $$(LIB_INCLUDES) $$(MBED_INCLUDES) -I$(GCC4MBED_DIR)/mri -c $$< -o $$@

    $$(LIB_DEVELOP_DIR)/%.o : $$(LIB_DIR)/%.S
		@echo Assembling $$<
		$(Q) $(MKDIR) $$(call convert-slash,$$(dir $$@)) $(QUIET)
		$(Q) $(GCC) $$(ASM_FLAGS) $$(LIB_INCLUDES) $$(MBED_INCLUDES) -I$(GCC4MBED_DIR)/mri -c $$< -o $$@

    $$(LIB_RELEASE_DIR)/%.o : $$(LIB_DIR)/%.c
		@echo Compiling $$<
		$(Q) $(MKDIR) $$(call convert-slash,$$(dir $$@)) $(QUIET)
		$(Q) $(GCC) $$(C_FLAGS) $$(LIB_INCLUDES) $$(MBED_INCLUDES) -I$(GCC4MBED_DIR)/mri -c $$< -o $$@

    $$(LIB_RELEASE_DIR)/%.o : $$(LIB_DIR)/%.cpp
		@echo Compiling $$<
		$(Q) $(MKDIR) $$(call convert-slash,$$(dir $$@)) $(QUIET)
		$(Q) $(GPP) $$(CPP_FLAGS) $$(LIB_INCLUDES) $$(MBED_INCLUDES) -I$(GCC4MBED_DIR)/mri -c $$< -o $$@

    $$(LIB_RELEASE_DIR)/%.o : $$(LIB_DIR)/%.s
		@echo Assembling $$<
		$(Q) $(MKDIR) $$(call convert-slash,$$(dir $$@)) $(QUIET)
		$(Q) $(GCC) $$(ASM_FLAGS) $$(LIB_INCLUDES) $$(MBED_INCLUDES) -I$(GCC4MBED_DIR)/mri -c $$< -o $$@

    $$(LIB_RELEASE_DIR)/%.o : $$(LIB_DIR)/%.S
		@echo Assembling $$<
		$(Q) $(MKDIR) $$(call convert-slash,$$(dir $$@)) $(QUIET)
		$(Q) $(GCC) $$(ASM_FLAGS) $$(LIB_INCLUDES) $$(MBED_INCLUDES) -I$(GCC4MBED_DIR)/mri -c $$< -o $$@

endef
define clean_user_lib #,lib_dir
	@echo Cleaning $1/Debug
	$(Q) $(REMOVE_DIR) $(call convert-slash,$1/Debug) $(QUIET)
	@echo Cleaning $1/Develop
	$(Q) $(REMOVE_DIR) $(call convert-slash,$1/Develop) $(QUIET)
	@echo Cleaning $1/Release
	$(Q) $(REMOVE_DIR) $(call convert-slash,$1/Release) $(QUIET)

endef
define add_user_lib #,lib_dir
    # Make sure that library directory doesn't have trailing slash and remove any '!' prefix.
    LIB_DIR := $(patsubst !%,%,$(patsubst %/,%,$1))

    # Library name is based on the directory name.
    LIB_NAME := $$(notdir $$(LIB_DIR))

    # Release and Debug target library paths.
    LIB_DEBUG_DIR   := $$(LIB_DIR)/Debug/$(MBED_DEVICE)
    DEBUG_LIB       := $$(LIB_DEBUG_DIR)/lib$$(LIB_NAME).a
    LIB_DEVELOP_DIR := $$(LIB_DIR)/Develop/$(MBED_DEVICE)
    DEVELOP_LIB     := $$(LIB_DEVELOP_DIR)/lib$$(LIB_NAME).a
    LIB_RELEASE_DIR := $$(LIB_DIR)/Release/$(MBED_DEVICE)
    RELEASE_LIB     := $$(LIB_RELEASE_DIR)/lib$$(LIB_NAME).a

    # Append to main project's list of user libraries.
    ifeq "$(GCC4MBED_TYPE)" "Debug"
        USER_LIBS_FULL += $$(DEBUG_LIB)
    endif
    ifeq "$(GCC4MBED_TYPE)" "Develop"
        USER_LIBS_FULL += $$(DEVELOP_LIB)
    endif
    ifeq "$(GCC4MBED_TYPE)" "Release"
        USER_LIBS_FULL += $$(RELEASE_LIB)
    endif
endef


# Recurse and find all mbed-os and main application folders once from this main makefile.
# This list of folders will be filtered later based on TARGET_* and FEATURE_* settings for each device.
RAW_MBED_DIRS := $(call recurse_dir,$(MBED_SRC_ROOT))
RAW_MAIN_DIRS := $(call recurse_dir,$(SRC))


# Rules for building all of the desired device targets
.PHONY: all clean clean-libs clean-mbed clean-all deploy help
all: $(DEVICES)
clean: $(addsuffix -clean,$(DEVICES))
clean-libs:
	$(foreach i,$(patsubst !%,%,$(USER_LIBS)),$(call clean_user_lib,$i))
clean-mbed:
	@echo Cleaning $(MBED_SRC_ROOT)/Debug
	$(Q) $(REMOVE_DIR) $(call convert-slash,$(MBED_SRC_ROOT)/Debug) $(QUIET)
	@echo Cleaning $(MBED_SRC_ROOT)/Develop
	$(Q) $(REMOVE_DIR) $(call convert-slash,$(MBED_SRC_ROOT)/Develop) $(QUIET)
	@echo Cleaning $(MBED_SRC_ROOT)/Release
	$(Q) $(REMOVE_DIR) $(call convert-slash,$(MBED_SRC_ROOT)/Release) $(QUIET)
clean-all: clean clean-libs clean-mbed
deploy: LPC1768-deploy


# Determine supported devices by looking at *-device.mk makefiles.
ALL_DEVICE_MAKEFILES := $(wildcard $(GCC4MBED_DIR)/build/*-device.mk)
ALL_DEVICES          := $(patsubst $(GCC4MBED_DIR)/build/%-device.mk,%,$(ALL_DEVICE_MAKEFILES))


# Help rule lets users know what the other available rules are.
define help_device #,device
    help::
		@echo "   $1"

endef

help::
	@echo "The following make rules are supported by this makefile:"
	@echo ''
	@echo " all - Incremental build."
	@echo " clean - Deletes the application's output binaries so that next "
	@echo "         \"make all\" will perform a clean build of those binaries."
	@echo " clean-libs - Deletes the output binaries in the user library folders"
	@echo "              so that next \"make all\" will perform a clean build of"
	@echo "              those binaries."
	@echo " clean-mbed - Deletes the output binaries in the mbed library folders"
	@echo "              so that next \"make all\" will perform a clean build of"
	@echo "              those binaries."
	@echo " clean-all - Deletes all output binaries so that next \"make all\" will"
	@echo "             perform a clean build of everything."
	@echo " deploy - Uses GCC4MBED_DEPLOY environment variable to copy firmware to"
	@echo "          device once it has been built."
	@echo ''
	@echo " A device name such as LPC1768 can be specified on the make command"
	@echo " line to just build binaries for that device. There are also rules like"
	@echo " LPC1768-deploy and LPC1768-clean to deploy or clean LPC1768 binaries"
	@echo " only. The supported devices include:"

$(foreach i,$(ALL_DEVICES),$(eval $(call help_device,$i)))


# Populate the MBED_IGNORE and MBED2_IGNORE variables by including mbed-ignore.mk
include $(GCC4MBED_DIR)/build/mbed-ignore.mk


# Include makefiles that know how to build each of the supported device types.
include $(ALL_DEVICE_MAKEFILES)
