#
# avr-uart - UART module for AVR microcontrollers
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

PROJECT_PREFIX := AVR_UART

PROJECT_ROOT := $(realpath ../../)

LIB_DIR := $(PROJECT_ROOT)/lib

INCLUDE_DIRS := $(realpath .)

include $(PROJECT_ROOT)/avr-uart.mk

TARGET_NAME = main
TARGET = $(TARGET_NAME).elf
TARGET_HEX = $(TARGET_NAME).hex

include $(PROJECT_ROOT)/Makefile.common

$(foreach d, $(DEP_LIBS_MODULES), $(eval $(call MAKE_DEP_LIB_DIR_VAR,$(d))))
$(foreach d, $(DEP_LIBS_MODULES), $(eval $(call MAKE_DEP_ROOT_DIR_VAR,$(d))))

-include $(foreach d, $(DEP_LIBS_MODULES), $(LIB_DIR)/$(d)/$(d).mk)

LIB_NAME = uart
LIB = lib$(LIB_NAME).a

DEP_SOURCES := $(wildcard *.c)
DEP_OBJECTS = $(DEP_SOURCES:.c=.o)
DEP_C_DEPS = $(DEP_SOURCES:.c=.d)
DEP_PREPROCESSOR_OUTPUTS = $(DEP_SOURCES:.c=.i)
DEP_ASSEMBLY_OUTPUTS = $(DEP_SOURCES:.c=.s)
DEP_STACK_USAGE_REPORTS = $(DEP_SOURCES:.c=.su)
DEP_ASSEMBLY_LISTINGS = $(DEP_SOURCES:.c=.lst)

SOURCES = $(TARGET_NAME).c
OBJECTS = $(SOURCES:.c=.o)
C_DEPS = $(SOURCES:.c=.d)
PREPROCESSOR_OUTPUTS = $(SOURCES:.c=.i)
ASSEMBLY_OUTPUTS = $(SOURCES:.c=.s)
STACK_USAGE_REPORTS = $(SOURCES:.c=.su)
ASSEMBLY_LISTINGS = $(SOURCES:.c=.lst)

all: $(TARGET_HEX) size

$(OBJECTS): $(SOURCES) $(INCLUDE_HEADERS)

$(LIB_DIR)/$(LIB):
	$(MAKE) -C $(PROJECT_ROOT)/src

$(TARGET): $(OBJECTS) $(LIB_DIR)/$(LIB) | $(DEP_LIBS_MODULES)
	$(COMPILE) -o $@ $(OBJECTS) $(LDFLAGS) $(LDLIBS)

# TODO: add EEPROM section
$(TARGET_HEX): $(TARGET)
	$(AVR_OBJCOPY) -j .text -j .data -O ihex $^ $@

flash: all
	$(AVRDUDE) -U flash:w:$(TARGET_HEX):i

install: flash

fuse:
	$(AVRDUDE) $(FUSES)

# TODO: flash bootloader
load: all
	@echo Nothing to do

clean:
	rm -f \
		$(OBJECTS) \
		$(C_DEPS) \
		$(PREPROCESSOR_OUTPUTS) \
		$(ASSEMBLY_OUTPUTS) \
		$(ASSEMBLY_LISTINGS) \
		$(STACK_USAGE_REPORTS) \
		$(TARGET_HEX) \
		$(TARGET)
