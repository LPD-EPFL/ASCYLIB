ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/lf-bst_ellen
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

test.o: bst_ellen.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

bst_ellen_test2.o: bst_ellen.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/bst_ellen_test2.o bst_ellen_test2.c

main: measurements.o ssalloc.o  bst_ellen.o test.o 
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/bst_ellen.o  $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

fairness: measurements.o ssalloc.o  bst_ellen.o bst_ellen_test2.o 
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/bst_ellen.o  $(BUILDIR)/bst_ellen_test2.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS)
