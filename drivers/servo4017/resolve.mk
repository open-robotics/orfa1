DEFINES += -DHAVE_SERVO
INCLUDE_DIRS += -Idrivers/servo4017

SRC += drivers/servo4017/servo_driver.c
SRC += drivers/servo4017/servo_interrupts.S

