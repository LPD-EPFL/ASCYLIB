ROOT = ../..

include $(ROOT)/common/Makefile.common

#TEST_FILE = test_simple.c

BINS = $(BINDIR)/bl-queue
PROF = $(ROOT)/src

.PHONY:	all clean

all:	main

measurements.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/measurements.o $(PROF)/measurements.c

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

bl-queue.o: 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/bl-queue.o bl-queue.c

test.o: bl-queue.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o  bl-queue.o test.o 
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/bl-queue.o  $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS)
