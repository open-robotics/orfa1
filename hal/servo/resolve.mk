# -*- Makefile -*-
ifeq ($(PLATFORM),OR_AVR_M32_D)
	LLD = gpio
else
	LLD = 4017
	HAL_SERVO_CMD = yes
endif

ifeq ($(HAL_SERVO_CMD),"yes")
	DEFINES += -DHAL_WITH_SERVO_CMD
	INCLUDE_DIRS += -I${ORFA}/hal/servo
	SRC += ${ORFA}/hal/servo/servo_cmd_lld.c
endif

INCLUDE_DIRS += -I${ORFA}/hal/servo/${LLD}

SRC += ${ORFA}/hal/servo/${LLD}/servo_lld.c

