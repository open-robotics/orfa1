PLATFORM=OR-AVR-M32-D

DEBUG = 1
MCU = atmega32
F_CPU = 7372800UL
BAUD = 115200
DEFINES = -DBAUD=B$(BAUD)

ifeq ($(PLATFORM),OR-AVR-M32-D)
	drv_src = $(wildcard drivers/*.c)
	drv_hdr = $(wildcard drivers/*.h)
	DEFINES += -DOR_AVR_M32_D
endif

MCU_FLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -DAVR_IO
CROSS_COMPILE_GCC = avr-
CROSS_COMPILE_BIN = avr-

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
SIZE = $(CROSS_COMPILE_BIN)size

CFLAGS = -std=gnu99 -W -Wall -pedantic -Wstrict-prototypes -Wundef \
-funsigned-char -funsigned-bitfields -ffunction-sections -fdata-sections \
-fpack-struct -fshort-enums -ffreestanding -Os -g -I. $(MCU_FLAGS) $(DEBUG_) $(DEFINES)
LDFLAGS = -Wl,--relax -Wl,--gc-sections

target = orfa
src = $(wildcard *.c) $(drv_src)
hdr = $(wildcard *.h) $(drv_hdr)
obj = $(src:.c=.o)
elf = $(target).elf
sre = $(target).srec
hex = $(target).hex
lss = $(target).lss
map = $(target).map
gdbinit = $(target).gdb
tag = tags

serialgatelib = serialgate/libserialgate.a
corelib = core/libcore.a

all: $(elf)
	$(SIZE) $(elf)

$(elf): $(obj) $(src) $(hdr) $(serialgatelib) $(corelib)
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) $(serialgatelib) $(corelib) -Wl,-Map,$(map) -o $(elf)

$(serialgatelib):
	$(MAKE) -C serialgate MCU_FLAGS="$(MCU_FLAGS)" BAUD="$(BAUD)" DEBUG="$(DEBUG)"

$(corelib):
	$(MAKE) -C core MCU_FLAGS="$(MCU_FLAGS)" DEBUG="$(DEBUG)"

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
	$(MAKE) -C serialgate clean
	$(MAKE) -C core clean

.PHONY: docs
docs:
	doxygen

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

