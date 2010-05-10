# -*- Makefile -*-

## General settings
## ================

## Select your platform
## Now we support:
##  - OR-AVR-M32-D (default)
##  - OR-AVR-M128-S
##  - OR-AVR-M128-DS
PLATFORM = OR_AVR_M32_D
#PLATFORM = OR_AVR_M128_S
#PLATFORM = OR_AVR_M128_DS

## Is it debug build? (default: nope)
#DEBUG = 1

## Serial baud rate (default: autodetection)
BAUD = B_AUTO
#BAUD = B2400
#BAUD = B4800
#BAUD = B9600
#BAUD = B14400
#BAUD = B19200
#BAUD = B28800
#BAUD = B38400
#BAUD = B57600
#BAUD = B76800
#BAUD = B115200

## I2C slave firmware
#I2C_SLAVE = 1

## SSC-32 compatible firmware
#ORC32 = 1


#--------------------------------------------------------------------
## Adapters
## ========
## Here you can enable additional adapters
## or disable adapter modules

# disable
#ADAPTERS := $(filter-out adc,$(ADAPTERS))
#ADAPTERS := $(filter-out spi,$(ADAPTERS))
#ADAPTERS := $(filter-out ports,$(ADAPTERS))
#ADAPTERS := $(filter-out motor,$(ADAPTERS))
#ADAPTERS := $(filter-out servo,$(ADAPTERS))

# enable
#ADAPTERS += cannon
#ADAPTERS += turret


#--------------------------------------------------------------------
## Defines
## =======

## Use protocol version 1.0
#DEFINES += -DSG_PROTOCOL_V1_0

## Disable interrupt driven serial input
#DEFINES += -DHAL_SERIAL_NISR


#--------------------------------------------------------------------
## Programmer
## ==========
## Avrdude programmer flags used by `make program`

##- AVR Dragon (default)
#PROGRAMMER = dragon_isp
#PROGRAMMER_PORT = usb

##- Bootloader
##  AVR910 compatible w/ 115200 baud rate
#PROGRAMMER = avr109 -b115200
#PROGRAMMER_PORT = /dev/ttyUSB0

