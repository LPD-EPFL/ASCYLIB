ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS  = $(BINDIR)/sq-ht
LLREP = lists
PROF = $(ROOT)/src

.PHONY:	all clean

all:	main

measurements.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/measurements.o $(PROF)/measurements.c

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

ll-intset.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/intset.o $(LLREP)/intset.c

seq.o: ll-intset.o 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/seq.o $(LLREP)/seq.c

linkedlist-seq.o: ll-intset.o seq.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/linkedlist-seq.o $(LLREP)/linkedlist-seq.c

hashtable-seq.o: ll-intset.o seq.o linkedlist-seq.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/hashtable-seq.o hashtable-seq.c

test.o: ll-intset.o seq.o linkedlist-seq.o hashtable-seq.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o ll-intset.o seq.o linkedlist-seq.o hashtable-seq.o test.o
	$(CC) $(CFLAGS)  $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/intset.o $(BUILDIR)/seq.o $(BUILDIR)/linkedlist-seq.o $(BUILDIR)/hashtable-seq.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	rm -f $(BINS)
