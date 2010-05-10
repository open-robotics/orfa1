# -*- makefile -*-
# HAL Test Makefile

target = hal-servo

ORFA = ../../..
#PLATFORM = OR_AVR_M32_D
PLATFORM = OR_AVR_M128_DS
HAL = servo
#MCU_FLAGS = -mmcu=atmega32 -DF_CPU=7372800UL
MCU_FLAGS = -mmcu=atmega128 -DF_CPU=7372800UL

CROSS_COMPILE_GCC = avr-
CROSS_COMPILE_BIN = avr-

CC = $(CROSS_COMPILE_GCC)gcc
LD = $(CROSS_COMPILE_BIN)ld
OBJCOPY = $(CROSS_COMPILE_BIN)objcopy
OBJDUMP = $(CROSS_COMPILE_BIN)objdump
SIZE = $(CROSS_COMPILE_BIN)size 

INCLUDE_DIRS = 

CFLAGS = -std=gnu99 -Os -I${ORFA} $(INCLUDE_DIRS) $(MCU_FLAGS)
LDFLAGS = -Wl,--relax -Wl,--gc-sections

DEFINES = -D$(PLATFORM)

SRC = hal-servo.c

include ${ORFA}/hal/resolve.mk

OBJS = $(patsubst %.c,%.o,$(SRC))

all: $(target).hex
	$(SIZE) $(target).elf

$(target).hex: $(target).elf
	$(OBJCOPY) -j .text -j .data -O ihex $(target).elf $(target).hex
	chmod -x $(target).hex $(target).elf

$(target).elf: $(OBJS) $(LIBS_RULES)
	$(CC) $(CFLAGS) -o $(target).elf $(OBJS) $(LIBS)

ram_size: $(target).elf
	readelf -s $< | grep OBJECT | awk '{ SUM += $$3 } END { print SUM }'

%.o: %.c
	$(CC) $(DEFINES) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) \
		$(target).hex $(target).elf

force: clean all

