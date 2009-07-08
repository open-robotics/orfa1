# -*- Makefile -*-

DEFINES += -DHAVE_SERVO
INCLUDE_DIRS += -Idrivers/servo_gpio

SRC += drivers/servo_gpio/servo_driver.c
