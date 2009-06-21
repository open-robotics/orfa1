DEFINES += -DHAVE_SERVO -DUSE_EEPROM
INCLUDE_DIRS += -Idrivers/servo4017

SRC += drivers/servo4017/servo_driver.c
SRC += drivers/servo4017/servo_interrupts.S

