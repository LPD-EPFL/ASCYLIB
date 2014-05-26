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

linkedlist.o: 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/linkedlist.o linkedlist.c

timnat.o: linkedlist.h linkedlist.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/timnat.o $(LIST_IMPL).c

intset.o: linkedlist.h timnat.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/intset.o intset.c

test.o: linkedlist.h timnat.h intset.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o linkedlist.o timnat.o timnat.o intset.o test.o $(TMILB)
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/linkedlist.o $(BUILDIR)/timnat.o $(BUILDIR)/intset.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS)
