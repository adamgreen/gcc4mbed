# Copyright 2011 Adam Green (http://mbed.org/users/AdamGreen/)
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
# Makefile to install external dependencies like mbed and FATFileSystem
# libraries from mbed.
ifeq "$(MAKE)" "cs-make"
SVN=win32\svn
MOVE=@echo Skipping move
DELTREE=-rd /s /q
PATCH=win32\applydiff --binary
QUIET=>nul 2>nul & exit 0
SHELL=cmd.exe
else
SVN=svn
MOVE=mv
DELTREE=rm -f -r
PATCH=patch
QUIET=> /dev/null 2>&1 ; exit 0
endif

.PHONY: install install_mbed patch_mbed

install: install_mbed patch_mbed

install_mbed:
	$(DELTREE) mbed $(QUIET)
	$(SVN) checkout --revision 43  http://mbed.org/projects/libraries/svn/mbed/export/trunk mbed

patch_mbed:
	$(DELTREE) mbed/LPC2368 $(QUIET)
	$(DELTREE) mbed/LPC11U24 $(QUIET)
	$(DELTREE) mbed/LPC1768/ARM $(QUIET)
	$(DELTREE) mbed/LPC1768/GCC_CR $(QUIET)
	$(DELTREE) mbed/LPC1768/GCC_CS $(QUIET)
	$(DELTREE) mbed/LPC1768/uARM $(QUIET)
	