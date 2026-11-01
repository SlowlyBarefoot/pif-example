# Build rules shared by the Windows examples. An example sets NAME and PIF_SRC, then includes this file.
#
#   make          build the example
#   make run      build and run it, Ctrl+C to stop
#   make clean    remove build output
#
# pif is looked for next to pif-example. Give PIF=<path> when it is somewhere else.
# The compiler defaults to "zig cc", as in the tests of pif. MinGW-w64 works too, e.g. "make CC=gcc".
# It runs under cmd.exe (mingw32-make) as well as under a POSIX shell (Git Bash, MSYS2).

ifeq ($(origin CC),default)
  CC := zig cc
endif

PIF     ?= ../../../pif
# The banner of the examples prints __DATE__ and __TIME__, which zig cc rejects by default.
CFLAGS  ?= -std=c99 -O2 -g -Wall -Wextra -Wno-unused-parameter -Wno-date-time
# The pif sources take pif_conf.h of the example. An absolute path keeps the cache of zig cc from giving one
# example the objects of another, built with the same relative command line.
INC     := -I$(CURDIR) -I.. -I$(PIF)/include
LIBS    := -lwinmm
BUILD   := build

# cmd.exe keeps the quotes that a POSIX shell strips.
ifeq ($(shell echo "q"),"q")
  MKDIR = if not exist $(subst /,\,$1) mkdir $(subst /,\,$1)
  RMDIR = if exist $(subst /,\,$1) rmdir /s /q $(subst /,\,$1)
  RUN   = $(subst /,\,$1)
else
  MKDIR = mkdir -p $1
  RMDIR = rm -rf $1
  RUN   = ./$1
endif

SRC     := $(wildcard *.c) ../host.c $(addprefix $(PIF)/source/,$(PIF_SRC))
OBJ     := $(addprefix $(BUILD)/,$(notdir $(SRC:.c=.o)))
TARGET  := $(BUILD)/$(NAME).exe

vpath %.c . .. $(sort $(dir $(addprefix $(PIF)/source/,$(PIF_SRC))))

.PHONY: all run clean

all: $(TARGET)

run: $(TARGET)
	$(call RUN,$(TARGET))

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

$(BUILD)/%.o: %.c pif_conf.h app_port.h ../host.h | $(BUILD)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

$(BUILD):
	$(call MKDIR,$@)

clean:
	$(call RMDIR,$(BUILD))
