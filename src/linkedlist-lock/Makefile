ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/lb-ll
PROF = $(ROOT)/src

.PHONY:	all clean

TEST_FILE = test_simple.c
ifeq ($(TEST),old)
	TEST_FILE = test.c
endif

all:	main

measurements.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/measurements.o $(PROF)/measurements.c

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

htlock.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/htlock.o $(PROF)/htlock.c

clh.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/clh.o $(PROF)/clh.c

linkedlist-lock.o: 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/linkedlist-lock.o linkedlist-lock.c

lazy.o: linkedlist-lock.h linkedlist-lock.o htlock.o clh.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/lazy.o lazy.c	 

coupling.o: linkedlist-lock.h linkedlist-lock.o clh.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/coupling.o coupling.c

intset.o: linkedlist-lock.h coupling.h lazy.h clh.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/intset.o intset.c

test.o: linkedlist-lock.h coupling.h lazy.h intset.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o linkedlist-lock.o coupling.o lazy.o intset.o test.o htlock.o clh.o
	$(CC) $(CFLAGS)l $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/htlock.o $(BUILDIR)/clh.o $(BUILDIR)/linkedlist-lock.o $(BUILDIR)/lazy.o $(BUILDIR)/coupling.o $(BUILDIR)/intset.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS) 

clean:
	rm -f $(BINS)
