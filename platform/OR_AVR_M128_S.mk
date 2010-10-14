# -*- Makefile -*-

BOARD_NAME = 'OR-AVR-M128-S'

MCU = atmega128
F_CPU = 7372800UL
BAUD = B_AUTO

ADAPTERS = ports adc servo

# default programmer -- preinstalled bootloader
PROGRAMMER = avr109 -b115200
PROGRAMMER_PORT = /dev/ttyUSB0

