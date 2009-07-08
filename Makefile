target = orfa

# $(PLATFORM) hack
-include local_config.mk

ifeq "$(PLATFORM)"  ""
    PLATFORM = OR_AVR_M32_D
endif

CONFIG_FILE = platform/$(PLATFORM).mk

CROSS_COMPILE_GCC =
CROSS_COMPILE_BIN =

AS = $(CROSS_COMPILE_BIN)as
CC = $(CROSS_COMPILE_GCC)gcc
CPLUSPLUS = $(CROSS_COMPILE_GCC)g++
LD = $(CROSS_COMPILE_BIN)ld
AR = $(CROSS_COMPILE_BIN)ar
OBJCOPY = $(CROSS_COMPILE_BIN)objcopy
OBJDUMP = $(CROSS_COMPILE_BIN)objdump
SIZE = $(CROSS_COMPILE_BIN)size 

INCLUDES = 
INCLUDE_DIRS = 

CFLAGS = -std=gnu99 -I. $(INCLUDE_DIRS) -Wall -Os -Wstrict-prototypes  -Werror $(MCU_FLAGS) -g \
		 -funsigned-char -funsigned-bitfields -ffunction-sections -fdata-sections \
		 -fmerge-all-constants -fstrict-aliasing -fpack-struct -fshort-enums -ffreestanding
ASFLAGS = -I. $(INCLUDE_DIRS) $(MCU_FLAGS) -xassembler-with-cpp
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

PROGRAMMER = dragon_isp
PROGRAMMER_PORT = usb

include $(CONFIG_FILE)
-include local_config.mk
include resolve.mk

ifeq "$(DEBUG)" ""
    DEFINES += -DNDEBUG
else
    DEFINES += -DDEBUG=$(DEBUG)
endif

OBJS = $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SRC))))

all: $(target).hex
	$(SIZE) $(target).elf

$(target).hex: $(target).elf
	$(OBJCOPY) -j .text -j .data -O ihex $(target).elf $(target).hex
	chmod -x $(target).hex $(target).elf

$(target).elf: $(OBJS) $(LIBS_RULES)
	$(CC) $(CFLAGS) -o $(target).elf $(OBJS) $(LIBS)

$(target).cof: $(target).elf
	$(COFFCONVERT) -O coff-ext-avr $< $(target).cof


%.o : %.c

%.o : %.cpp

%.o : %.S


%.o: %.c $(CONFIG_FILE) local_config.mk
	$(CC) $(DEFINES) $(INCLUDES) $(CFLAGS) -c -o $@ $<

%.o: %.cpp $(CONFIG_FILE) local_config.mk
	$(CPLUSPLUS) $(DEFINES) $(INCLUDES) $(CFLAGS) -c -o $@ $<

%.o: %.S $(CONFIG_FILE) local_config.mk
	$(CC) $(DEFINES) $(INCLUDES) $(ASFLAGS) -c -o $@ $<

%::
	touch local_config.mk


clean:
	rm -rf $(shell find -name '*.o' -o -name '*.a'  -o -name ${target}.hex -o -name $(target).elf)
	rm -f doxygen.log tags

docs:
	doxygen

force: clean all

program: $(target).hex
	avrdude -p $(MCU) -P $(PROGRAMMER_PORT) -c $(PROGRAMMER) -U flash:w:$<

tags:
	ctags -o $@ -R $(shell find -name '*.c' -o -name '*.h')

# debugging
include debug.mk
