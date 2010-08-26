# -*- Makefile -*-

MCU_FLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU)
CROSS_COMPILE_GCC = avr-
CROSS_COMPILE_BIN = avr-

ifeq ($(DEBUG),0)
	DEBUG_ = -DNDEBUG
else
	DEBUG_ = -DDEBUG=$(DEBUG)
endif
ifeq ($(DEBUG),2)
    MCU_FLAGS =
    CROSS_COMPILE_GCC =
    CROSS_COMPILE_BIN =
endif

include eterm/resolve.mk
include core/resolve.mk
include adapters/resolve.mk
include hal/resolve.mk
include lib/resolve.mk

