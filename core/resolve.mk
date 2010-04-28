# -*- Makefile -*-

CORELIB_TARGET = core/libcore.a
CORELIB_SRC = $(filter-out ${ORFA}/core/test.c,$(wildcard ${ORFA}/core/*.c))
CORELIB_OBJS = $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(CORELIB_SRC))))

LIBS += $(CORELIB_TARGET)
LIBS_RULES += $(CORELIB_TARGET)($(CORELIB_OBJS))
#DEFINES +=
