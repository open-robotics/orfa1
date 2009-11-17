# -*- Makefile -*-

DEFINES += -DI2C_SLAVE -DI2C_MASTER
INCLUDE_DIRS += -Ii2c_slave
SRC += i2c_slave/i2c.c

INCLUDES += -includei2c_slave/i2c.h
