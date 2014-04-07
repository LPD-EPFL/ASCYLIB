ROOT = ../..

include $(ROOT)/common/Makefile.common

ifeq ($(RO_FAIL),0)
	CFLAGS += -DRO_FAIL=0
	SUFFIX = _no_ro
else
	CFLAGS += -DRO_FAIL=1
endif

BINS  = $(BINDIR)/lb-ht_pugh$(BIN_SUFFIX)$(SUFFIX)
LLREP = lists
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

ll-intset.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/intset.o $(LLREP)/intset.c

pugh.o: ll-intset.o htlock.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/pugh.o $(LLREP)/pugh.c	 

linkedlist-lock.o: ll-intset.o pugh.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/linkedlist-lock.o $(LLREP)/linkedlist-lock.c

hashtable-lock.o: ll-intset.o pugh.o linkedlist-lock.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/hashtable-lock.o hashtable-lock.c

test.o: ll-intset.o pugh.o linkedlist-lock.o hashtable-lock.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o ll-intset.o pugh.o linkedlist-lock.o hashtable-lock.o test.o htlock.o
	$(CC) $(CFLAGS)  $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/htlock.o $(BUILDIR)/clh.o $(BUILDIR)/intset.o $(BUILDIR)/pugh.o $(BUILDIR)/linkedlist-lock.o $(BUILDIR)/hashtable-lock.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	rm -f $(BINS)
