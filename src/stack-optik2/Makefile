ROOT = ../..

include $(ROOT)/common/Makefile.common

ALGO_O = stack-optik.o
ALGO_C = stack-optik.c

BINS = "$(BINDIR)/lb-st_optik2$(BIN_SUFFIX)"
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

stack-lock.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/stack-lock.o stack-lock.c

$(ALGO_O): stack-lock.h 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/$(ALGO_O) $(ALGO_C)

intset.o: stack-lock.h stack-lock.h htlock.o clh.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/intset.o intset.c

test.o: stack-lock.h stack-lock.h intset.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o stack-lock.o $(ALGO_O) intset.o test.o clh.o
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/htlock.o $(BUILDIR)/clh.o $(BUILDIR)/stack-lock.o $(BUILDIR)/$(ALGO_O) $(BUILDIR)/intset.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS) *.o
