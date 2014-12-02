ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/lf-ht_harris
LLREP = lists
PROF = $(ROOT)/src

.PHONY:	all clean

all:	main

LIST_IMPL = harris1

measurements.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/measurements.o $(PROF)/measurements.c

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

linkedlist.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/linkedlist.o $(LLREP)/linkedlist.c

harris.o: $(LLREP)/linkedlist.h linkedlist.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/harris.o $(LLREP)/$(LIST_IMPL).c

ll-intset.o: $(LLREP)/linkedlist.h linkedlist.o harris.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ll-intset.o $(LLREP)/intset.c

hashtable.o: $(LLREP)/linkedlist.h linkedlist.o harris.o ll-intset.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/hashtable.o hashtable.c

intset.o: $(LLREP)/linkedlist.h harris.o hashtable.o 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/intset.o intset.c

test.o: linkedlist.o harris.o intset.o hashtable.o intset.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o linkedlist.o harris.o intset.o hashtable.o intset.o test.o 
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/linkedlist.o $(BUILDIR)/harris.o $(BUILDIR)/ll-intset.o $(BUILDIR)/hashtable.o $(BUILDIR)/intset.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS)
