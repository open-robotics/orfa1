ifneq "$(HAVE_MOTOR)" ""
    include drivers/motor/resolve.mk
endif
ifneq "$(HAVE_SPI)" ""
    include drivers/spi/resolve.mk
endif
ifneq "$(HAVE_PORTS)" "" # Yikes !
    include drivers/ports/resolve.mk
endif

ifneq "$(HAVE_SERVO_4017)" ""
    include drivers/servo4017/resolve.mk
endif

