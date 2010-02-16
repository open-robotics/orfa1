# -*- Makefile -*-

#------------------------------------------------
# General settings

# Select your platform
# Now we support:
#   OR-AVR-M32-D (default)
#   OR-AVR-M128-S
#
PLATFORM = OR_AVR_M32_D
#PLATFORM = OR_AVR_M128_S

# Is debug build? (default nope)
#DEBUG = 1

# Baudrate
BAUD = B_AUTO
#BAUD = B9600
#BAUD = B115200

#I2C_SLAVE = 1

#------------------------------------------------
# Drivers
# Here you can enable or disable driver modules

#ADAPTERS := $(filter-out adc,$(DRIVERS))
#ADAPTERS := $(filter-out spi,$(DRIVERS))
#ADAPTERS := $(filter-out ports,$(DRIVERS))
#ADAPTERS := $(filter-out motor,$(DRIVERS))
#ADAPTERS := $(filter-out servo,$(DRIVERS))

#ADAPTERS += cannon
#ADAPTERS += turret

#------------------------------------------------
# Defines

# Use protocol version 1.0
#DEFINES += -DSG_PROTOCOL_V1_0

# Disable interrupt driven serial input
#DEFINES += -DSG_DISABLE_IRQ

