target = orfa
ORFA = .
DEBUG = 0

# $(PLATFORM) hack
-include local_config.mk

ifeq "$(PLATFORM)"  ""
    PLATFORM = OR_AVR_M128_S
endif

CONFIG_FILE = ${ORFA}/platform/$(PLATFORM).mk

CROSS_COMPILE_GCC =
CROSS_COMPILE_BIN =

AS = $(CROSS_COMPILE_BIN)as
CC = $(CROSS_COMPILE_GCC)gcc
CXX = $(CROSS_COMPILE_GCC)g++
LD = $(CROSS_COMPILE_BIN)ld
AR = $(CROSS_COMPILE_BIN)ar
OBJCOPY = $(CROSS_COMPILE_BIN)objcopy
OBJDUMP = $(CROSS_COMPILE_BIN)objdump
SIZE = $(CROSS_COMPILE_BIN)size 

INCLUDES = 
INCLUDE_DIRS = 

CFLAGS = -std=gnu99 -I${ORFA} $(INCLUDE_DIRS) -Wall -Os -Wstrict-prototypes  -Werror $(MCU_FLAGS) -g \
		 -funsigned-char -funsigned-bitfields -ffunction-sections -fdata-sections \
		 -fmerge-all-constants -fstrict-aliasing -fpack-struct -fshort-enums -ffreestanding
ASFLAGS = -I${ORFA} $(INCLUDE_DIRS) $(MCU_FLAGS) -xassembler-with-cpp
LDFLAGS = -Wl,--relax -Wl,--gc-sections

COFFCONVERT=$(OBJCOPY) --debugging -O coff-ext-avr \
	--change-section-address .data-0x800000 \
	--change-section-address .bss-0x800000 \
	--change-section-address .noinit-0x800000 \
	--change-section-address .eeprom-0x810000

DEFINES = -D$(PLATFORM)

LIBS = 
LIBS_RULES = 
SRC = main.c

include $(CONFIG_FILE)
-include local_config.mk
include resolve.mk

OBJS = $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SRC))))

all: $(target).hex
	$(SIZE) $(target).elf

$(target).hex: $(target).elf
	$(OBJCOPY) -j .text -j .data -O ihex $(target).elf $(target).hex
	chmod -x $(target).hex $(target).elf

$(target).elf: $(OBJS) $(LIBS_RULES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(target).elf $(OBJS) $(LIBS)

$(target).cof: $(target).elf
	$(COFFCONVERT) -O coff-ext-avr $< $(target).cof

$(target).lss: $(target).elf
	$(OBJDUMP) -d $< > $@

%.o: %.c $(CONFIG_FILE) local_config.mk
	$(CC) $(DEFINES) $(INCLUDES) $(CFLAGS) -c -o $@ $<

%.o: %.cpp $(CONFIG_FILE) local_config.mk
	$(CXX) $(DEFINES) $(INCLUDES) $(CFLAGS) -c -o $@ $<

%.o: %.S $(CONFIG_FILE) local_config.mk
	$(CC) $(DEFINES) $(INCLUDES) $(ASFLAGS) -c -o $@ $<

ram_size: $(target).elf
	readelf -s $< | awk '$$4 ~ /OBJECT/ { SUM += $$3 } END { print SUM }'

config: local_config.mk

local_config.mk:
	cp ${ORFA}/doc/local_config.mk ${ORFA}/

program: $(target).hex
	avrdude -p $(MCU) -P $(PROGRAMMER_PORT) -c $(PROGRAMMER) -U flash:w:$<

clean:
	rm -f $(LIBS) $(OBJS) \
		$(target).hex $(target).elf $(target).cof $(target).lss \
		doxygen.log tags

deepclean:
	rm -f $(shell find -name '*.o' -o -name '*.a' \
		-o -name '*.hex' -o -name '*.elf' \
		-o -name '*.cof' -o -name '*.lss') \
		doxygen.log tags
	rm -rf ${ORFA}/doc/doxygen/html ${ORFA}/doc/doxygen/latex

docs:
	doxygen

docs_pdf: docs
	make -C ${ORFA}/doc/doxygen/latex

force: clean all

tags:
	ctags -o $@ -R $(shell find -name '*.c' -o -name '*.h')

# debugging
include debug.mk
