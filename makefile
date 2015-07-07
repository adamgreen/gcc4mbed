# Copyright 2013 Adam Green (http://mbed.org/users/AdamGreen/)
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
#
# Directories to be built
DIRS=samples
DIRSCLEAN = $(addsuffix .clean,$(DIRS))

# Set VERBOSE make variable to 1 to output all tool commands.
VERBOSE?=0
ifeq "$(VERBOSE)" "0"
Q=@
else
Q=
endif

# Make sure that cmd.exe is used as shell on Windows.
ifeq "$(OS)" "Windows_NT"
SHELL=cmd.exe
endif

all: $(DIRS)

clean: $(DIRSCLEAN)

$(DIRS):
	@echo Building $@
	$(Q) $(MAKE) -C $@ all

$(DIRSCLEAN): %.clean:
	@echo Cleaning $*
	$(Q) $(MAKE) -C $*  clean

.PHONY: all clean $(DIRS) $(DIRSCLEAN)
