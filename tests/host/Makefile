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

PROJECT_ROOT := $(realpath ../../)

INCLUDE_DIRS += $(realpath .)

include $(PROJECT_ROOT)/avr-uart.mk

BUILD_HOST := 1

include $(PROJECT_ROOT)/Makefile.common

SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

COMPILE = gcc -g -Wall -Wextra $(INCLUDE) $(CFLAGS) -DF_CPU=$(CLOCK)
ifneq ($(strip $(RELEASE)),)
COMPILE = gcc -Wall -Wextra $(INCLUDE) $(CFLAGS) -DF_CPU=$(CLOCK)
endif

TARGET := uart_test.elf

%.o: %.c
	$(COMPILE) -c $< -o $@

$(OBJECTS): $(SOURCES) $(INCLUDE_HEADERS)

all: $(TARGET)

$(TARGET) : $(OBJECTS)
	$(COMPILE) -o $(TARGET) $(OBJECTS)

run_test: $(TARGET)
	$(CURDIR)/$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)
