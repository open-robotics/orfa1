DEBUG = 1
MCU = atmega32
F_CPU = 7372800UL
BAUD = 115200

MCU_FLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -DAVR_IO
CROSS_COMPILE_GCC = avr-
CROSS_COMPILE_BIN = avr-

DEFINES = -DBAUD=B$(BAUD)

ifeq ($(DEBUG),0)
	DEBUG_ = -DNDEBUG
else
	DEBUG_ = -DDEBUG=$(DEBUG)
endif
ifeq ($(DEBUG),2)
    MCU_FLAGS =
    CROSS_COMPILE_GCC =
    CROSS_COMPILE_BIN =
endif

AS = $(CROSS_COMPILE_BIN)as
CC = $(CROSS_COMPILE_GCC)gcc
CPLUSPLUS = $(CROSS_COMPILE_GCC)g++
LD = $(CROSS_COMPILE_BIN)ld
AR = $(CROSS_COMPILE_BIN)ar
OBJCOPY = $(CROSS_COMPILE_BIN)objcopy
OBJDUMP = $(CROSS_COMPILE_BIN)objdump

CFLAGS = -std=gnu99 -W -Wall -pedantic -Wstrict-prototypes -Wundef \
-funsigned-char -funsigned-bitfields -ffunction-sections -fdata-sections \
-fpack-struct -fshort-enums -ffreestanding -Os -g -I. $(MCU_FLAGS) $(DEBUG_) $(DEFINES)
LDFLAGS = -Wl,--relax -Wl,--gc-sections

target = serialgate.a
src = $(filter-out test.c, $(wildcard *.c))
hdr = $(wildcard *.h)
obj = $(patsubst %.c,%.o,$(src))
tag = tags

all: $(target)

$(target): $(target)($(obj))

test: $(target) test.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(target) test.c -o $@

.PHONY: tags
tags:
	ctags -o $(tag) -R $(src) $(hdr)

.PHONY: clean
clean:
	$(RM) $(target) $(obj) test test.o $(tag)
