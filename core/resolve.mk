CORELIB_TARGET = core/libcore.a
CORELIB_SRC = $(filter-out core/test.c,$(wildcard core/*.c))
CORELIB_OBJS = $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(CORELIB_SRC))))

LIBS += $(CORELIB_TARGET)
LIBS_RULES += $(CORELIB_TARGET)($(CORELIB_OBJS))
DEFINES += -DBAUD=$(BAUD)


