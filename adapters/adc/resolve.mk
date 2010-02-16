# -*- Makefile -*-

DEFINES += -DHAVE_ADC
INCLUDE_DIRS += -I${ORFA}/adapters/adc

HAL += adc

SRC += ${ORFA}/adapters/adc/adc_i2c.c
