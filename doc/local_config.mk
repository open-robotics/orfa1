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

#------------------------------------------------
# Drivers
# Here you can enable or disable driver modules

#DRIVERS := $(filter-out adc,$(DRIVERS))
#DRIVERS := $(filter-out spi,$(DRIVERS))
#DRIVERS := $(filter-out ports,$(DRIVERS))
#DRIVERS := $(filter-out motor,$(DRIVERS))
#DRIVERS := $(filter-out servo_gpio,$(DRIVERS))
#DRIVERS := $(filter-out servo4017,$(DRIVERS))

#DRIVERS += cannon
#DRIVERS += turret

#------------------------------------------------
# Defines

# Use protocol version 1.1
#DEFINES += -DSG_PROTOCOL_V1_1

# Use interrupt driven serial input
DEFINES += -DSG_ENABLE_IRQ

