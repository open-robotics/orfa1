# -*- Makefile -*-

#INCLUDE_DIRS += -I${ORFA}/hal

include $(foreach hal,$(sort $(HAL)), ${ORFA}/hal/$(hal)/resolve.mk)

