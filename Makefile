target = orfa

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

INCLUDES = 
INCLUDE_DIRS = 

CFLAGS = -std=gnu99 -I. $(INCLUDE_DIRS) -Wall -Os -Wstrict-prototypes  -Werror $(MCU_FLAGS) -g
ASFLAGS = -I. $(INCLUDE_DIRS) $(MCU_FLAGS) -xassembler-with-cpp

COFFCONVERT=$(OBJCOPY) --debugging \
   -O coff-ext-avr \
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

ifeq "$(DEBUG)" ""
    DEFINES += -DNDEBUG
endif

OBJS = $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SRC))))

all: $(target).hex

$(target).hex: $(target).elf
	$(OBJCOPY) -j .text -j .data -O ihex $(target).elf $(target).hex
	chmod -x $(target).hex $(target).elf

$(target).elf: $(OBJS) $(LIBS_RULES)
	$(CC) $(CFLAGS) -o $(target).elf \
		$(OBJS) $(LIBS)

$(target).cof : $(target).elf
	$(COFFCONVERT) -O coff-ext-avr $< $(target).cof


%.o : %.c

%.o : %.cpp

%.o : %.S


%.o: %.c $(CONFIG_FILE)
	$(CC) $(DEFINES) $(INCLUDES) $(CFLAGS) -c -o $@ $<

%.o: %.cpp $(CONFIG_FILE)
	$(CPLUSPLUS) $(DEFINES) $(INCLUDES) $(CFLAGS) -c -o $@ $<

%.o: %.S $(CONFIG_FILE)
	$(CC) $(DEFINES) $(INCLUDES) $(ASFLAGS) -c -o $@ $<


clean:
	rm -rf `find -name '*.o' -o -name '*.a'  -o -name ${target}.hex -o -name $(target).elf `
	rm -f doxygen.log

docs:
	doxygen

force: clean all

