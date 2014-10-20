ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/lf-ll_smr_hart
PROF = $(ROOT)/src

.PHONY:	all clean

all:	main

LIST_IMPL = harris

measurements.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/measurements.o $(PROF)/measurements.c

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

smr.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/smr.o $(PROF)/smr.c

# linkedlist.o: 
# 	$(CC) $(CFLAGS) -c -o $(BUILDIR)/linkedlist.o linkedlist.c

# harris.o: linkedlist.h linkedlist.o
# 	$(CC) $(CFLAGS) -c -o $(BUILDIR)/harris.o $(LIST_IMPL).c

# intset.o: linkedlist.h harris.h
# 	$(CC) $(CFLAGS) -c -o $(BUILDIR)/intset.o intset.c

list.o: list.h node.h smr.o
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/list.o list_lf_smr.c

test.o: list.h 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o smr.o list.o test.o $(TMILB)
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/smr.o $(BUILDIR)/list.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS)
