# -*- Makefile -*-

ORC32LIB_TARGET = ${ORFA}/orc32/liborc32.a
ORC32LIB_SRC = $(wildcard ${ORFA}/orc32/*.c)
ORC32LIB_OBJS = $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(ORC32LIB_SRC))))

LIBS += $(ORC32LIB_TARGET)
LIBS_RULES += $(ORC32LIB_TARGET)($(ORC32LIB_OBJS))
DEFINES += -DBAUD=$(BAUD)

INCLUDES += -includeorc32/orc32.h

target = orc32
HAL = serial servo
ADAPTERS =
