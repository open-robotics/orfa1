# -*- Makefile -*-
ifeq ($(PLATFORM),OR-AVR-M32-D)
	LLD = gpio
else
	LLD = 4017
endif

HALINC += ${ORFA}/hal/servo \
		  ${ORFA}/hal/servo/${LLD}

HALSRC += ${ORFA}/hal/servo/${LLD}/servo_lld.c
