#
# avr-uart - UART module for AVRmicrocontrollers
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

PROJECT_ROOT := $(CURDIR)

# This variable is defined at the top-level (project root level) Makefile to
# include the required upper directories containing header files. 
# It needs to be defined on the command line to run make from any lower
# directory.
INCLUDE_DIRS := config include port
export INCLUDE_DIRS := $(foreach i, $(INCLUDE_DIRS), $(realpath $(i)))

LIB_SRC_DIR = $(CURDIR)/src

LIB_DIR := $(PROJECT_ROOT)/lib

TESTS_DIR := $(PROJECT_ROOT)/tests

all: build-lib build-tests


$(LIB_DIR):
	mkdir -p $(LIB_DIR)

build-lib: | $(LIB_DIR)
	$(MAKE) -C $(LIB_SRC_DIR)

build-tests:
	$(MAKE) -C $(TESTS_DIR)

size:
	$(AVR_SIZE) -C --mcu=$(DEVICE) main.elf

help:
	$(MAKE) -C $(LIB_SRC_DIR) help

clean:
	$(MAKE) -C $(LIB_SRC_DIR) clean
	$(MAKE) -C $(TESTS_DIR) clean
	rm -rf $(LIB_DIR)
