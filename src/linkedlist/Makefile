ROOT = ../..

include $(ROOT)/common/Makefile.common

LIST_V=1
# LIST_V=

BINS = $(BINDIR)/lf-ll$(LIST_V)
PROF = $(ROOT)/src

.PHONY:	all clean

all:	main

LIST_IMPL = harris$(LIST_V)

measurements.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/measurements.o $(PROF)/measurements.c

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

linkedlist.o: 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/linkedlist.o linkedlist.c

harris.o: linkedlist.h linkedlist.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/harris.o $(LIST_IMPL).c

intset.o: linkedlist.h harris.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/intset.o intset.c

test.o: linkedlist.h harris.h intset.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o linkedlist.o harris.o harris.o intset.o test.o $(TMILB)
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/linkedlist.o $(BUILDIR)/harris.o $(BUILDIR)/intset.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS)
