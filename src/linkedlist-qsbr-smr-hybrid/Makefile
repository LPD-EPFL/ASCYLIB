ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/lf-ll_hybrid
PROF = $(ROOT)/src

.PHONY:	all clean

all:	main

LIST_IMPL = harris

measurements.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/measurements.o $(PROF)/measurements.c

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

qsbr_smr_hybrid.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/qsbr_smr_hybrid.o $(PROF)/qsbr_smr_hybrid.c

htlock.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/htlock.o $(PROF)/htlock.c

list.o: list.h node.h qsbr_smr_hybrid.o htlock.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/list.o list_hybrid.c

test.o: list.h 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o qsbr_smr_hybrid.o list.o test.o htlock.o $(TMILB)
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/qsbr_smr_hybrid.o $(BUILDIR)/list.o $(BUILDIR)/htlock.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS)
