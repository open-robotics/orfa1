# -*- Makefile -*-

DEFINES += -DHAVE_ADC
INCLUDE_DIRS += -I${ORFA}/drivers/adc
HAL += adc
SRC += ${ORFA}/drivers/adc/adc_driver.c
