ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/wf-ll-timnat
PROF = $(ROOT)/src

.PHONY:	all clean

all:	main

LIST_IMPL = timnat

measurements.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/measurements.o $(PROF)/measurements.c

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

timnat.o: 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/timnat.o $(LIST_IMPL).c

test.o: timnat.h 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o timnat.o test.o $(TMILB)
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/timnat.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS)
