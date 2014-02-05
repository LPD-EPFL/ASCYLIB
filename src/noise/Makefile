ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/noise

.PHONY:	all clean

all:	main

noise.o: noise.c
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/noise.o noise.c

main: noise.o
	$(CC) $(CFLAGS) $(BUILDIR)/noise.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS)