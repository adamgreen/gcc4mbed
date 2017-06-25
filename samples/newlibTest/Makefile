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
PROJECT         := newlibTest
DEVICES         := LPC1768 KL25Z K64F
GCC4MBED_DIR    := ../..

GCC4MBED_TYPE   := Release
MRI_ENABLE      := 1
MRI_UART        := MRI_UART_MBED_USB MRI_UART_BAUD=230400
MBED_CONFIG_H   := mbed_config.h

NEWLIB_NANO     := 0
NO_FLOAT_SCANF  := 1
NO_FLOAT_PRINTF := 1

include $(GCC4MBED_DIR)/build/gcc4mbed.mk

GCOV_FILTERS    = $(MBED_SRC_ROOT)/platform/mbed_retarget.cpp
