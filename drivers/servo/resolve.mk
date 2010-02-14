# -*- Makefile -*-

DEFINES += -DHAVE_SERVO
INCLUDE_DIRS += -I${ORFA}/drivers/servo

HAL += servo

SRC += ${ORFA}/drivers/servo/servo_driver.c
	   
