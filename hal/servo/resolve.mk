# -*- Makefile -*-
ifeq ($(PLATFORM),OR_AVR_M32_D)
	LLD = gpio
else
	LLD = 4017
endif

INCLUDE_DIRS += -I${ORFA}/hal/servo/${LLD}

SRC += ${ORFA}/hal/servo/${LLD}/servo_lld.c

