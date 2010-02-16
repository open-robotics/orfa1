# -*- Makefile -*-

include $(foreach adapter,$(sort $(ADAPTERS)), adapters/$(adapter)/resolve.mk)

