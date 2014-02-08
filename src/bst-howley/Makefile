ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/lf-bst-howley
PROF = $(ROOT)/src

.PHONY:	all clean

all:	main

measurements.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/measurements.o $(PROF)/measurements.c

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

bst_howley.o: 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/bst_howley.o bst_howley.c

bst_howley_test.o: bst_howley.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/bst_howley_test.o bst_howley_test.c

bst_howley_test_scalability.o: bst_howley.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/bst_howley_test_scalability.o bst_howley_test_scalability.c

correctness: measurements.o ssalloc.o  bst_howley.o bst_howley_test.o 
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/bst_howley.o  $(BUILDIR)/bst_howley_test.o -o $(BINS) $(LDFLAGS)

test.o: bst_howley.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o  bst_howley.o test.o 
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/bst_howley.o  $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS)
