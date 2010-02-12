# -*- Makefile -*-

INCLUDE_DIRS += -I${ORFA}/hal

include $(foreach hal,$(HAL), ${ORFA}/hal/$(hal)/resolve.mk)

