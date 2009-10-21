# -*- Makefile -*-
#
# Also known as OR-AVR-M64-S
#

MCU = atmega128
F_CPU = 7372800UL
BAUD = B115200

DRIVERS = ports spi adc servo4017
DEFINES += -DADC_ISR
