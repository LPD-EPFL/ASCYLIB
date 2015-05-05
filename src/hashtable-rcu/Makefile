ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS  = $(BINDIR)/lf-ht_rcu
PROF = $(ROOT)/src

LDFLAGS += -L$(URCU_PATH)/lib -lurcu-cds -lurcu-qsbr -lurcu-signal -lurcu
CFLAGS += -I$(URCU_PATH)/include

.PHONY:	all clean

all:	main

measurements.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/measurements.o $(PROF)/measurements.c

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

test.o: $(TEST_FILE)
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: 	test.o ssalloc.o measurements.o
	$(CC) $(CFLAGS) $(BUILDIR)/ssalloc.o $(BUILDIR)/measurements.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	rm -f $(BINS)
