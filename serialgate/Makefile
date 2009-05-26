AS = as
CC = gcc
CPLUSPLUS = g++
LD = ld
AR = ar
OBJCOPY = objcopy
OBJDUMP = objdump

CFLAGS = -std=gnu99 -W -Wall -Werror -pedantic -Wstrict-prototypes -Wundef \
-funsigned-char -funsigned-bitfields -ffunction-sections -fdata-sections \
-fpack-struct -fshort-enums -ffreestanding -Os -g -I. 
LDFLAGS = -Wl,--relax -Wl,--gc-sections

target = test
src = $(filter-out usart.c,$(wildcard *.c))
hdr = $(wildcard *.h)
obj = $(patsubst %.c,%.o,$(src))
tag = tags

all: $(target)

$(target): $(src) $(hdr)
	$(CC) $(CFLAGS) $(LDFLAGS) $(src)  -o $@

.PHONY: tags
tags:
	ctags -o $(tag) -R $(src) $(hdr)

.PHONY: clean
clean:
	$(RM) $(target) $(obj) test test.o $(tag)
