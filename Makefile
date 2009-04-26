#CC = avr-gcc
CFLAGS += -std=gnu99 -W -Wall -pedantic -Wstrict-prototypes -Wundef #-Werror
CFLAGS += -funsigned-char -funsigned-bitfields -ffunction-sections -fpack-struct -fshort-enums #-finline-limit=20
CFLAGS += -ffreestanding -Os -g #-gdwarf-2
#CFLAGS += -DHAVE_CONFIG_H
CFLAGS += -DOR_M32_D # Open Robotics OR-M32-D
#CFLAGS += -DF_CPU=$(F_CPU) -mmcu=$(MCU)
CFLAGS += -I.
#LDFLAGS = -mmcu=$(MCU)
LDFLAGS += -Wl,--relax -Wl,--gc-sections
#OBJCOPY = avr-objcopy
#OBJDUMP = avr-objdump
#SIZE = avr-size



target = i2c-gate
src = $(wildcard *.c)
hdr = $(wildcard *.h)
obj = $(src:.c=.o)
elf = $(target).elf
sre = $(target).srec
hex = $(target).hex
lss = $(target).lss
map = $(target).map
tag = tags

ver = $(shell sed -ne '/define *VERSION_STRING /{s/.*"\(.*\)".*/\1/p; q;}' serial-command.h)
dst = i2c-gate-v${ver}


all: $(elf) 
	#$(lss) $(hex)
	#$(SIZE) $(elf)

$(elf): $(obj) $(src) $(hdr) $(lufalib)
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) $(lufalib) -Wl,-Map,$(map) -o $(elf)

%.sre: %.elf
	$(OBJCOPY) -j .text -j .data -O srec $< $@

%.lss: %.elf
	$(OBJDUMP) -D $< > $@

%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@


.PHONY: tags
tags:
	ctags -o $(tag) -R $(src) $(hdr)

.PHONY: clean
clean:
	$(RM) $(elf) $(obj) $(sre) $(lss) $(map) $(hex) $(tag)
	$(RM) -rf "${dst}".zip "${dst}"

.PHONY: dist
dist: $(elf) $(hex)
	#svn export . "$(dst)"
	hg archive "$(dst)"
	cp $^ "$(dst)"
	$(RM) -rf "$(dst)"/contrib "$(dst)"/test $(lss)
	zip -r "$(dst)".zip "$(dst)"
	$(RM) -rf "$(dst)"
