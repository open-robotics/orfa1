# -*- Makefile -*-

## General settings
## ================

## Select your platform
## Now we support:
##  - OR-AVR-M32-D
##  - OR-AVR-M128-S (default)
##  - OR-AVR-M128-DS
#PLATFORM = OR_AVR_M32_D
#PLATFORM = OR_AVR_M128_S
#PLATFORM = OR_AVR_M128_DS

## Is it debug build? (default: nope)
#DEBUG = 1

## Serial baud rate (default: autodetection)
#BAUD = B_AUTO
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

## I2C slave address (default 0x7F)
## NOTE: 0x7F corresponds to 0xFE when writing and OxFF when reading
## (0x7F * 2 = 0xFE)
#I2C_SLAVE_ADDRESS = 0x77


## Programmer
## ==========
## Avrdude programmer flags used by `make program`
## NOTE: for OR-AVR-M128-S and OR-AVR-M128-DS default programmer
## is preprogremmed avr910 compatible bootloader.

##- AVR Dragon (ISP mode)
#PROGRAMMER = dragon_isp
#PROGRAMMER_PORT = usb

##- Bootloader
##  AVR910 compatible w/ 115200 baud rate
#PROGRAMMER = avr109 -b115200
#PROGRAMMER_PORT = /dev/ttyUSB0

##- Arduino bootloader
##  STK500 compatible protocol w/ 115200 baud rate
#PROGRAMMER = stk500 -b115200
#PROGRAMMER_PORT = /dev/ttyUSB0


## I2C Adapters
## ============
## Here you can enable additional I2C adapters
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


## Defines
## =======

## Disable interrupt driven serial input
#DEFINES += -DHAL_SERIAL_NISR

