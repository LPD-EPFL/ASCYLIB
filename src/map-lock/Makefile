ROOT = ../..

include $(ROOT)/common/Makefile.common

ifeq ($(RO_FAIL),0)
	CFLAGS += -DRO_FAIL=0
	SUFFIX = _no_ro
else
	CFLAGS += -DRO_FAIL=1
endif

BINS = $(BINDIR)/lb-map$(BIN_SUFFIX)$(SUFFIX)
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

set-lock.o: 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/set-lock.o set-lock.c

set-lockb.o: set-lock.h set-lock.o htlock.o clh.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/set-lockb.o set-lockb.c	 

intset.o: set-lock.h set-lockb.h clh.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/intset.o intset.c

test.o: set-lock.h set-lockb.h intset.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o set-lock.o set-lockb.o intset.o test.o htlock.o clh.o
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/htlock.o $(BUILDIR)/clh.o $(BUILDIR)/set-lock.o $(BUILDIR)/set-lockb.o $(BUILDIR)/intset.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS) 

clean:
	rm -f $(BINS)
