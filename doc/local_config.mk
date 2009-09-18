# -*- Makefile -*-

#############################
# General

## Select platform
# We support:
#   OR-AVR-M32-D (default)
#   OR-AVR-M128-S
#PLATFORM = OR_AVR_M128_S
PLATFORM = OR_AVR_M32_D

## Debug build? (default nope)
#DEBUG = 1

#############################
# Drivers
# Here you can enable or disable driver modules

#HAVE_ADC =
#HAVE_PORTS =
#HAVE_SPI =
#HAVE_MOTOR =
#HAVE_SERVO_GPIO =
#HAVE_SERVO_4017 =
#HAVE_CANON = 1
#HAVE_TURRET = 1

#############################
# Defines section

## Use protocol version 1.1
#DEFINES += -DSG_PROTOCOL_V1_1

## Use interrupt driven serial input
#DEFINES += -DSG_ENABLE_IRQ
