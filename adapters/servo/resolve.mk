# -*- Makefile -*-

DEFINES += -DHAVE_SERVO
INCLUDE_DIRS += -I${ORFA}/adapters/servo

HAL += servo

SRC += ${ORFA}/adapters/servo/servo_i2c.c

