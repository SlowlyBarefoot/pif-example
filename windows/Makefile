# Builds every Windows example, i.e. every folder here that has a Makefile.
#
#   make              build all examples
#   make clean        remove the build output of all examples
#   make <example>    build one example, e.g. make ex_task1
#
# Variables given on the command line (CC, PIF, CFLAGS) are passed to each example. A relative PIF is taken
# from the folder of the example, not from here.

PROJECTS := $(patsubst %/Makefile,%,$(wildcard */Makefile))

.PHONY: all clean $(PROJECTS)

all: $(PROJECTS)

$(PROJECTS):
	$(MAKE) -C $@

clean: $(addprefix clean-,$(PROJECTS))

clean-%:
	$(MAKE) -C $* clean
