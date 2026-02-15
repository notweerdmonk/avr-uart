#
# avr-uart - UART module for AtMEGA microcontrollers
# Copyright (C) 2026 notweerdmonk
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
#

.DEFAULT_GOAL = all

PROJECT_ROOT := $(realpath ../../)

include $(PROJECT_ROOT)/Makefile.common

ifeq ($(INCLUDE_DIRS),)
INCLUDE_DIRS := $(PROJECT_ROOT)/config $(PROJECT_ROOT)/include $(PROJECT_ROOT)/port
INCLUDE_DIRS := $(foreach i, $(INCLUDE_DIRS), $(realpath $(i)))
endif

INCLUDE_DIRS += $(realpath .)

INCLUDE += $(foreach i, $(INCLUDE_DIRS), -I$(realpath $(i)))
INCLUDE_HEADERS = $(foreach dir, $(abspath $(INCLUDE_DIRS)), $(wildcard $(dir)/*.h))

TARGET_NAME = main
TARGET = $(TARGET_NAME).elf
TARGET_HEX = $(TARGET_NAME).hex

SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
C_DEPS = $(SOURCES:.c=.d)
PREPROCESSOR_OUTPUTS = $(SOURCES:.c=.i)
ASSEMBLY_OUTPUTS = $(SOURCES:.c=.s)
STACK_USAGE_REPORTS = $(SOURCES:.c=.su)
ASSEMBLY_LISTINGS = $(SOURCES:.c=.lst)

LIB_DIR := $(PROJECT_ROOT)/lib

LDFLAGS += -L$(LIB_DIR)

LDLIBS = -luart

all: $(TARGET_HEX) size

$(OBJECTS): $(SOURCES) $(INCLUDE_HEADERS)

build-lib:
	$(MAKE) -C $(PROJECT_ROOT) $@

$(TARGET): $(OBJECTS) build-lib
	$(COMPILE) -o $@ $(OBJECTS) $(LDFLAGS) $(LDLIBS)

# TODO: add EEPROM section
$(TARGET_HEX): $(TARGET)
	$(AVR_OBJCOPY) -j .text -j .data -O ihex $^ $@

flash: all
	$(AVRDUDE) -U flash:w:main.hex:i

install: flash

fuse:
	$(AVRDUDE) $(FUSES)

# TODO: flash bootloader
load: all
	@echo Nothing to do

clean:
	rm -f $(OBJECTS) $(C_DEPS) $(PREPROCESSOR_OUTPUTS) $(ASSEMBLY_OUTPUT) \
		$(STACK_USAGE_REPORTS) $(ASSEMBLY_LISTINGS) $(TARGET_HEX) $(TARGET)
