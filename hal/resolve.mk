# -*- Makefile -*-

#INCLUDE_DIRS += -I${ORFA}/hal

include $(foreach hal,$(sort $(HAL)), ${ORFA}/hal/$(hal)/resolve.mk)

HALLIB_TARGET = hal/libhal.a
HALLIB_OBJS = $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(HAL_SRC))))

LIBS += $(HALLIB_TARGET)
LIBS_RULES += $(HALLIB_TARGET)($(HALLIB_OBJS))

