ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/lb-bst_ss
PROF = $(ROOT)/src

.PHONY:	all clean

all:	main

measurements.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/measurements.o $(PROF)/measurements.c

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

bst.o: 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/bst.o bst.c

bst_ss.o: bst.h bst.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/bst_ss.o bst_ss.c

intset.o: bst.h bst_ss.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/intset.o intset.c

test.o: bst.h bst_ss.h intset.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o bst.o bst_ss.o intset.o test.o
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/bst.o $(BUILDIR)/bst_ss.o $(BUILDIR)/intset.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS) 

clean:
	rm -f $(BINS)
