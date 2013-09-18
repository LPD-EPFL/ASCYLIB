ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/lf-bst
PROF = $(ROOT)/src

.PHONY:	all clean

all:	main

measurements.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/measurements.o $(PROF)/measurements.c

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

bst_ellen.o: 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/bst_ellen.o bst_ellen.c

bst_test.o: bst_ellen.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/bst_test.o bst_test.c

main: measurements.o ssalloc.o  bst_ellen.o bst_test.o 
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/bst_ellen.o  $(BUILDIR)/bst_test.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS)
