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
# "" — disable, 1 — enable

#HAVE_ADC =
#HAVE_PORTS =
#HAVE_SPI =
#HAVE_MOTOR =
#HAVE_SERVO_GPIO =
#HAVE_SERVO_4017 =
#HAVE_CANON = 1
#HAVE_TURRET = 1

#------------------------------------------------
# Defines

# Use protocol version 1.1
#DEFINES += -DSG_PROTOCOL_V1_1

# Use interrupt driven serial input
DEFINES += -DSG_ENABLE_IRQ

