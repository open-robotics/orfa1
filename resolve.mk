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

include core/resolve.mk
include drivers/resolve.mk
ifeq "$(I2C_SLAVE)" ""
include serialgate/resolve.mk
else
include i2c_slave/resolve.mk
endif

