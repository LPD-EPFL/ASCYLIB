ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/sq-bst_internal
PROF = $(ROOT)/src

.PHONY:	all clean

all:	main

measurements.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/measurements.o $(PROF)/measurements.c

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

bst-seq.o: 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/bst-seq.o bst-seq.c

seq.o: bst-seq.h bst-seq.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/seq.o seq.c

intset.o: bst-seq.h seq.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/intset.o intset.c

test.o: bst-seq.h seq.h intset.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o bst-seq.o seq.o intset.o test.o
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/bst-seq.o $(BUILDIR)/seq.o $(BUILDIR)/intset.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS) 

clean:
	rm -f $(BINS)
