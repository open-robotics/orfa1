# -*- Makefile -*-

ETERMLIB_TARGET = ${ORFA}/eterm/libeterm.a
ETERMLIB_SRC = ${ORFA}/eterm/eterm.c \
			   ${ORFA}/eterm/eterm_main.c \
			   ${ORFA}/eterm/sgparsers.c \
			   ${ORFA}/eterm/orc32parsers.c \
			   ${ORFA}/eterm/portparsers.c \
			   ${ORFA}/eterm/wdtparser.c

ifeq ($(PLATFORM),OR_AVR_M32_D)
	ETERMLIB_SRC += ${ORFA}/eterm/md2parsers.c
endif
ifeq ($(PLATFORM),OR_AVR_M128_DS)
	ETERMLIB_SRC += ${ORFA}/eterm/md2parsers.c
endif

ETERMLIB_OBJS = $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(ETERMLIB_SRC))))

LIBS += $(ETERMLIB_TARGET)
LIBS_RULES += $(ETERMLIB_TARGET)($(ETERMLIB_OBJS))
DEFINES += -DBAUD=$(BAUD)

HAL += serial i2c
