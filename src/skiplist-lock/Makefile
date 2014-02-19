ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/lb-sl$(BIN_SUFFIX)
PROF = $(ROOT)/src

.PHONY:	all clean

all:	main

measurements.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/measurements.o $(PROF)/measurements.c

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

htlock.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/htlock.o $(PROF)/htlock.c

clh.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/clh.o $(PROF)/clh.c

skiplist-lock.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/skiplist-lock.o skiplist-lock.c

optimistic.o: skiplist-lock.h 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/optimistic.o optimistic.c 

intset.o: skiplist-lock.h optimistic.h htlock.o clh.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/intset.o intset.c

test.o: skiplist-lock.h optimistic.h intset.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o skiplist-lock.o optimistic.o intset.o test.o clh.o
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/htlock.o $(BUILDIR)/clh.o $(BUILDIR)/skiplist-lock.o $(BUILDIR)/optimistic.o $(BUILDIR)/intset.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS) *.o
