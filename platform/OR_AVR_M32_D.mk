# -*- Makefile -*-

BOARD_NAME = 'OR-AVR-M32-D'

MCU = atmega32
F_CPU = 7372800UL
BAUD = B_AUTO

ADAPTERS = ports adc motor servo

# default programmer -- AVR Dragon
PROGRAMMER = dragon_isp
PROGRAMMER_PORT = usb

