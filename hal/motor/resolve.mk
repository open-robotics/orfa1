# -*- Makefile -*-

ifeq ($(PLATFORM),OR_AVR_M32_D)
	MLLD = m32
else ifeq ($(PLATFORM),OR_AVR_M16_DS)
	MLLD = m16
else
	MLLD = m128
endif

INCLUDE_DIRS += -I${ORFA}/hal/motor/${MLLD}

HAL_SRC += ${ORFA}/hal/motor/${MLLD}/motor_lld.c

