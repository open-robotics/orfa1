# -*- Makefile -*-

MCU_FLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -DAVR_IO
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

ifeq "$(I2C_SLAVE)" ""
	ifeq "$(ORC32)" ""
		include serialgate/resolve.mk
	else
		include orc32/resolve.mk
	endif
else
	include i2c_slave/resolve.mk
endif

include core/resolve.mk
include adapters/resolve.mk
include hal/resolve.mk
include lib/resolve.mk

