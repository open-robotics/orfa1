# -*- Makefile -*-

DEFINES += -DHAVE_ADC
INCLUDE_DIRS += -Idrivers/adc
SRC += drivers/adc/adc_driver.c #drivers/adc/adc_interrupts.S
