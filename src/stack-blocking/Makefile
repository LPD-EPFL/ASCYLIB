ROOT = ../..

include $(ROOT)/common/Makefile.common

#TEST_FILE = test_simple.c

BINS = $(BINDIR)/bl-stack
PROF = $(ROOT)/src

.PHONY:	all clean

all:	main

measurements.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/measurements.o $(PROF)/measurements.c

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

bl-stack.o: 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/bl-stack.o bl-stack.c

test.o: bl-stack.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o  bl-stack.o test.o 
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/bl-stack.o  $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS)
