# -*- Makefile -*-

DEFINES += -DHAVE_MOTOR
INCLUDE_DIRS += -I${ORFA}/drivers/motor

HAL += motor

SRC += ${ORFA}/drivers/motor/motor_driver.c
