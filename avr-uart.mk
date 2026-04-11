export AVR_UART_DEP_LIBS_MODULES := $(DEP_LIBS_MODULES) avr-utils avr-portable

ifeq ($(strip $(AVR_UART_ROOT_DIR)),)
AVR_UART_ROOT_DIR := $(PROJECT_ROOT)
endif

ifeq ($(strip $(AVR_UART_LIB_DIR)),)
AVR_UART_LIB_DIR := $(LIB_DIR)
endif

INCLUDE_DIRS += \
$(foreach d,config include port,$(AVR_UART_ROOT_DIR)/$(d)) \
$(foreach d, $(AVR_UART_DEP_LIBS_MODULES), $(AVR_UART_LIB_DIR)/$(d)/include) \
$(foreach d, $(AVR_UART_DEP_LIBS_MODULES), $(AVR_UART_LIB_DIR)/$(d)/port)

# This variable is defined at the top-level (project root level) Makefile to
# include the required upper directories containing header files.
# It needs to be defined on the command line to run make from any lower
# directory.
export AVR_UART_INCLUDE_DIRS := $(foreach i, $(INCLUDE_DIRS), $(realpath $(i)))
export AVR_UART_INCLUDE_HEADERS := $(foreach dir, $(abspath $(AVR_UART_INCLUDE_DIRS)), $(wildcard $(dir)/*.h))
export AVR_UART_INCLUDE := $(foreach i, $(AVR_UART_INCLUDE_DIRS), -I$(realpath $(i)))

override CFLAGS += $(AVR_UART_INCLUDE)

ifeq ($(strip $(BUILD_LIB)),)
LDFLAGS += -L$(AVR_UART_LIB_DIR)
LDLIBS += -luart
endif
