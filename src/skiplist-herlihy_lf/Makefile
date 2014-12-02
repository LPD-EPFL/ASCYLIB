ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/lf-sl_herlihy
PROF = $(ROOT)/src

CFLAGS += -O2

.PHONY:	all clean

all:	main

ALGO_O = herlihy.o
ALGO_C = herlihy.c
measurements.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/measurements.o $(PROF)/measurements.c

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

skiplist.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/skiplist.o skiplist.c

$(ALGO_O): skiplist.h 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/$(ALGO_O) $(ALGO_C)

intset.o: skiplist.h herlihy.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/intset.o intset.c

test.o: skiplist.h herlihy.h intset.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o skiplist.o $(ALGO_O) intset.o test.o 
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/skiplist.o $(BUILDIR)/$(ALGO_O) $(BUILDIR)/intset.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS)
