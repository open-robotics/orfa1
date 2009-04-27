DEV = 'pc'
DEBUG = 1 

#CC = avr-gcc
CFLAGS += -std=gnu99 -W -Wall -pedantic -Wstrict-prototypes -Wundef #-Werror
CFLAGS += -funsigned-char -funsigned-bitfields -ffunction-sections -fdata-sections -fpack-struct -fshort-enums #-finline-limit=20
CFLAGS += -ffreestanding -Os -g
CFLAGS += -I.
LDFLAGS += -Wl,--relax -Wl,--gc-sections
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = size

ifeq ($(DEV),avr)
    MCU = atmega32
	#MCU = atmega16
    F_CPU = 7372800UL
    CC = avr-gcc
    SIZE = avr-size
    CFLAGS += -DF_CPU=$(F_CPU) -mmcu=$(MCU) -gdwarf-2
    CFLAGS += -DAVR_IO
    LDFLAGS = -mmcu=$(MCU)
endif

ifeq ($(DEBUG),0)
	CFLAGS += -DNDEBUG
endif

target = i2c-gate
src = $(wildcard *.c)
hdr = $(wildcard *.h)
obj = $(src:.c=.o)
elf = $(target).elf
sre = $(target).srec
hex = $(target).hex
lss = $(target).lss
map = $(target).map
gdbinit = $(target).gdb
tag = tags

ver = $(shell sed -ne '/define *VERSION_STRING /{s/.*"\(.*\)".*/\1/p; q;}' serial-command.h)
dst = i2c-gate-v${ver}

ifeq ($(DEV),avr)
all: $(elf) $(lss) $(hex)
	$(SIZE) $(elf)
else
all: $(elf)
	$(SIZE) $(elf)
endif

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
	$(RM) $(elf) $(obj) $(sre) $(lss) $(map) $(hex) $(tag) $(gdbinit)
	$(RM) -rf "${dst}".zip "${dst}"

.PHONY: dist
dist: $(elf) $(hex)
	#svn export . "$(dst)"
	hg archive "$(dst)"
	cp $^ "$(dst)"
	$(RM) -rf "$(dst)"/contrib "$(dst)"/test $(lss)
	zip -r "$(dst)".zip "$(dst)"
	$(RM) -rf "$(dst)"
	
# programming
.PHONY: program
program: $(hex)
	avrdude -p $(MCU) -P usb -c dragon_isp -U flash:w:$<

# debugging-related targets
.PHONY: ddd
ddd: gdbinit
	ddd --debugger "avr-gdb -x $(gdbinit)"

.PHONY: gdbserver
gdbserver: gdbinit
	simulavr --device $(MCU) --gdbserver

gdbinit: $(gdbinit)

$(gdbinit): $(hex)
	@echo "file $(elf)" > $(gdbinit)	
	@echo "target remote localhost:1212" >> $(gdbinit)
	@echo "load"        >> $(gdbinit)
	@echo "break main"  >> $(gdbinit)
	@echo "continue"    >> $(gdbinit)
	@echo
	@echo "Use 'avr-gdb -x $(gdbinit)'"

