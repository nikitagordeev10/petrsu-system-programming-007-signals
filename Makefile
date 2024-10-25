CC      = gcc
CFLAGS  = -Wall -O3 -march=native -mtune=native
LD      = gcc
LDFLAGS = -s

.c.o:
	$(CC) $(CFLAGS) -c $<

ALL	= $(basename $(wildcard *.c))

all: ${ALL}

clean:
	rm -f ${ALL}
	rm -f *.o *~
	rm -f .depend

depend dep:
	$(CC) $(CFLAGS) -E -M -MG *.c > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
