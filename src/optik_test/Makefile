ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/optik_test$(OPTIK)
PROF = $(ROOT)/src

.PHONY:	all clean

all:	main

measurements.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/measurements.o $(PROF)/measurements.c

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

optik_test.o: optik_test.c
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/optik_test.o optik_test.c

main: measurements.o ssalloc.o optik_test.o
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/optik_test.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS)
