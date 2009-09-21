# -*- Makefile -*-

include $(foreach driver,$(DRIVERS), drivers/$(driver)/resolve.mk)

