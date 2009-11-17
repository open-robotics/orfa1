# -*- Makefile -*-

SERLIB_TARGET = serialgate/libserialgate.a
SERLIB_SRC = $(filter-out serialgate/test.c,$(wildcard serialgate/*.c))
SERLIB_OBJS = $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SERLIB_SRC))))

LIBS += $(SERLIB_TARGET)
LIBS_RULES += $(SERLIB_TARGET)($(SERLIB_OBJS))
DEFINES += -DBAUD=$(BAUD)

INCLUDES += -includeserialgate/i2c.h -includeserialgate/serialgate.h

