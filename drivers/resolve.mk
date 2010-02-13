# -*- Makefile -*-

include $(foreach driver,$(sort $(DRIVERS)), drivers/$(driver)/resolve.mk)

