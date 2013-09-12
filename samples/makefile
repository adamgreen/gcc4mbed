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
DIRS=HelloWorld\
     Blink\
     Ticker\
     StdIO\
     LocalFileSystem\
     FileTest\
     SdPerf\
     TCPSocket_HelloWorld\
     USBMouse\
     USBHostMouse\
     USBHostKeyboard\
     USBHostMSD\
     USBHostHub\
     USBHostSerial

DIRSCLEAN = $(addsuffix .clean,$(DIRS))
DIRS_LPC1768 = $(addsuffix .lpc1768,$(DIRS))
DIRS_LPC11U24 = $(addsuffix .lpc11u24,$(DIRS))
DIRS_KL25Z = $(addsuffix .kl25z,$(DIRS))

# Set VERBOSE make variable to 1 to output all tool commands.
VERBOSE?=0
ifeq "$(VERBOSE)" "0"
Q=@
else
Q=
endif

all: $(DIRS)

clean: $(DIRSCLEAN)

LPC1768: $(DIRS_LPC1768)
LPC11U24: $(DIRS_LPC11U24)
KL25Z: $(DIRS_KL25Z)

clean-all: $(DIRSCLEAN)
	$(Q) $(MAKE) -C HelloWorld LPC1768_MBED_clean LPC11U24_MBED_clean KL25Z_MBED_clean

$(DIRS):
	@echo Building $@
	$(Q) $(MAKE) -C $@ all

$(DIRSCLEAN): %.clean:
	$(Q) $(MAKE) -C $* clean

$(DIRS_LPC1768): %.lpc1768:
	$(Q) $(MAKE) -C $* LPC1768

$(DIRS_LPC11U24): %.lpc11u24:
	$(Q) $(MAKE) -C $* LPC11U24

$(DIRS_KL25Z): %.kl25z:
	$(Q) $(MAKE) -C $* KL25Z

.PHONY: all clean LPC1768 LPC11U24 KL25Z clean-all $(DIRS) $(DIRSCLEAN) $(DIRS_LPC1768) $(DIRS_LPC11U24) $(DIRS_KL25Z)
