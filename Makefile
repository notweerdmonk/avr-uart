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

PROJECT_ROOT := $(CURDIR)

LIB_SRC_DIR = $(CURDIR)/src

LIB_DIR := $(PROJECT_ROOT)/lib

TESTS_DIR := $(PROJECT_ROOT)/tests

BUILD_LIB := 1

include $(PROJECT_ROOT)/avr-uart.mk

all: build-lib build-tests

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

build-lib: | $(LIB_DIR)
	$(MAKE) -C $(LIB_SRC_DIR)

build-tests:
	$(MAKE) -C $(TESTS_DIR)

# Make distribution
dist:
	@make -s clean
	tar --exclude=.gdb* --exclude=peda-session-*.txt --exclude=.*.swp \
		--exclude=tags --exclude-backups --exclude-vcs --exclude-vcs-ignore -czvf \
		$(shell basename $(shell pwd)).tar.gz ./*

help:
	$(MAKE) -C $(LIB_SRC_DIR) help

clean:
	for dep_dir in $($(PROJECT_PREFIX)_DEP_LIBS_MODULES); do \
		$(MAKE) -C "$(LIB_DIR)/$${dep_dir}" clean; \
	done
	$(MAKE) -C $(LIB_SRC_DIR) clean
	$(MAKE) -C $(TESTS_DIR) clean
	rm -f $(LIB_DIR)/*.a
