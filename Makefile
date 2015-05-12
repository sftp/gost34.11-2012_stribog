#
# Copyright (c) 2013 by sftp
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted.
#
# There's ABSOLUTELY NO WARRANTY, express or implied.
#

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
