ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/lb-pq_alistarh_pugh
PROF = $(ROOT)/src

.PHONY:	all clean

all:	main

ALGO_O = alistarh_pugh.o
ALGO_H = alistarh_pugh.h
ALGO_C = alistarh_pugh.c
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

$(ALGO_O): skiplist-lock.h 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/$(ALGO_O) $(ALGO_C)

intPriorityQueue.o: skiplist-lock.h $(ALGO_H) htlock.o clh.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/intPriorityQueue.o intPriorityQueue.c

test.o: skiplist-lock.h $(ALGO_H) intPriorityQueue.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o skiplist-lock.o $(ALGO_O) intPriorityQueue.o test.o clh.o
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/htlock.o $(BUILDIR)/clh.o $(BUILDIR)/skiplist-lock.o $(BUILDIR)/$(ALGO_O) $(BUILDIR)/intPriorityQueue.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS)
