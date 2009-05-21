DEBUG = 1
MCU = atmega32
F_CPU = 7372800UL
BAUD = 115200
PLATFORM = -DOR_AVR_M32_D

drv_src = $(wildcard drivers/*.c)
drv_hdr = $(wildcard drivers/*.h)
