ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/lf-pq_alistarh
PROF = $(ROOT)/src

.PHONY:	all clean

all:	main

ALGO_O = alistarh.o
ALGO_H = alistarh.h
ALGO_C = alistarh.c
measurements.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/measurements.o $(PROF)/measurements.c

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

skiplist.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/skiplist.o skiplist.c

$(ALGO_O): skiplist.h 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/$(ALGO_O) $(ALGO_C)

intPriorityQueue.o: skiplist.h $(ALGO_H)
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/intPriorityQueue.o intPriorityQueue.c

test.o: skiplist.h $(ALGO_H) intPriorityQueue.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o skiplist.o $(ALGO_O) intPriorityQueue.o test.o 
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/skiplist.o $(BUILDIR)/$(ALGO_O) $(BUILDIR)/intPriorityQueue.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS)
