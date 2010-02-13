# -*- Makefile -*-

ifeq ($(PLATFORM),OR_AVR_M32_D)
	DEFINES += -DHAL_ADC_NISR
endif

INCLUDE_DIRS += -I${ORFA}/hal/adc
SRC += ${ORFA}/hal/adc/adc_lld.c
