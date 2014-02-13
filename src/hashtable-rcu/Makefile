ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS  = $(BINDIR)/lf-ht_rcu
LLREP = $(ROOT)/src/linkedlist-lock
PROF = $(ROOT)/src

URCU_PATH = /home/trigonak/code/urcu
LDFLAGS += -lurcu-cds -lurcu
CFLAGS += -I$(URCU_PATH)/

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

coupling.o: ll-intset.o 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/coupling.o $(LLREP)/coupling.c

lazy.o: ll-intset.o htlock.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/lazy.o $(LLREP)/lazy.c      

linkedlist-lock.o: ll-intset.o coupling.o lazy.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/linkedlist-lock.o $(LLREP)/linkedlist-lock.c

hashtable-lock.o: ll-intset.o coupling.o lazy.o linkedlist-lock.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/hashtable-lock.o hashtable-lock.c

test.o: $(TEST_FILE)
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: 	test.o ssalloc.o
	$(CC) $(CFLAGS) $(BUILDIR)/ssalloc.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	rm -f $(BINS)
