# -*- Makefile -*-

COMMONLIB_TARGET = ${ORFA}/lib/libcommon.a
COMMONLIB_SRC = $(wildcard ${ORFA}/lib/*.c)
COMMONLIB_OBJS = $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(COMMONLIB_SRC))))

LIBS += $(COMMONLIB_TARGET)
LIBS_RULES += $(COMMONLIB_TARGET)($(COMMONLIB_OBJS))

#INCLUDE_DIRS += -I${ORFA}/lib
