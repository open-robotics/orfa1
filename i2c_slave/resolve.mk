# -*- Makefile -*-

DEFINES += -DI2C_SLAVE -DI2C_MASTER
INCLUDE_DIRS += -Ii2c_slave
SRC += i2c_slave/i2c.c

INCLUDES += -includei2c_slave/i2c.h

ifeq "$(I2C_SLAVE_ADDRESS)" ""
    I2C_SLAVE_ADDRESS = 0x7F
endif

DEFINES += -DI2C_SLAVE_ADDRESS=$(I2C_SLAVE_ADDRESS)

