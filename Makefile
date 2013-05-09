V =
ifeq ($(strip $(V)),)
	E = @echo
	Q = @
else
	E = @\#
	Q =
endif
export E Q

CC = gcc

WARNINGS = -Wall
CFLAGS   = -O3 $(WARNINGS)

PROGS = stribog

default: all
all:    $(PROGS)

stribog: stribog.c stribog.h stribog_data.h types.h
	$(E) "  CC      " $@
	$(Q) ${CC} $(CFLAGS) stribog.c -o stribog

clean:
	$(E) "  CLEAN   " $(PROGS)
	$(Q) rm -f $(PROGS)
