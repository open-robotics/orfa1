# -*- Makefile -*-

ifeq ($(PLATFORM),OR_AVR_M32_D)
	DEFINES += -DHAL_ADC_NISR
endif

INCLUDE_DIRS += -I${ORFA}/hal/adc
HAL_SRC += ${ORFA}/hal/adc/adc_lld.c
