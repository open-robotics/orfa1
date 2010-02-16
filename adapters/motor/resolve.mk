# -*- Makefile -*-

DEFINES += -DHAVE_MOTOR
INCLUDE_DIRS += -I${ORFA}/adapters/motor

HAL += motor

SRC += ${ORFA}/adapters/motor/motor_i2c.c
