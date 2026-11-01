# Build rules shared by the Linux examples. An example sets NAME and PIF_SRC, then includes this file.
# Sources from outside pif go in EXT_SRC, with their include paths in EXT_INC.
#
#   make          build the example
#   make run      build and run it, Ctrl+C to stop
#   make clean    remove build output
#
# pif is looked for next to pif-example. Give PIF=<path> when it is somewhere else.
# The compiler defaults to gcc. Any C99 compiler works, e.g. "make CC=clang".

ifeq ($(origin CC),default)
  CC := gcc
endif

PIF     ?= ../../../pif
CFLAGS  ?= -std=c99 -O2 -g -Wall -Wextra -Wno-unused-parameter
# The pif sources take pif_conf.h of the example.
INC     := -I$(CURDIR) -I.. -I$(PIF)/include $(EXT_INC)
LIBS    := -lpthread
BUILD   := build

SRC     := $(wildcard *.c) ../host.c $(addprefix $(PIF)/source/,$(PIF_SRC)) $(EXT_SRC)
OBJ     := $(addprefix $(BUILD)/,$(notdir $(SRC:.c=.o)))
TARGET  := $(BUILD)/$(NAME)

vpath %.c . .. $(sort $(dir $(addprefix $(PIF)/source/,$(PIF_SRC)) $(EXT_SRC)))

.PHONY: all run clean

all: $(TARGET)

run: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

$(BUILD)/%.o: %.c $(wildcard *.h) ../host.h | $(BUILD)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

$(BUILD):
	mkdir -p $@

clean:
	rm -rf $(BUILD)
