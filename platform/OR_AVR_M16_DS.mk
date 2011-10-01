# -*- Makefile -*-

BOARD_NAME = 'OR-AVR-M16-DS'

MCU = atmega168
F_CPU = 16000000UL
BAUD = B_AUTO

ADAPTERS = ports adc motor servo

# default programmer -- preinstalled bootloader
PROGRAMMER = avr109 -b115200
PROGRAMMER_PORT = /dev/ttyUSB0

