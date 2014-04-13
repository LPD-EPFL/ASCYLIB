ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/lb-bst_tk
PROF = $(ROOT)/src

.PHONY:	all clean

all:	main

measurements.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/measurements.o $(PROF)/measurements.c

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

bst.o: 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/bst.o bst.c

bst_tk.o: bst.h bst.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/bst_tk.o bst_tk.c

intset.o: bst.h bst_tk.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/intset.o intset.c

test.o: bst.h bst_tk.h intset.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o bst.o bst_tk.o intset.o test.o
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/bst.o $(BUILDIR)/bst_tk.o $(BUILDIR)/intset.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS) 

clean:
	rm -f $(BINS)
