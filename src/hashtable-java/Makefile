ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS  = $(BINDIR)/lb-ht_java
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

concurrent_hash_map.o: concurrent_hash_map.c concurrent_hash_map.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/concurrent_hash_map.o concurrent_hash_map.c

test.o: 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o test.o htlock.o concurrent_hash_map.o
	$(CC) $(CFLAGS)  $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/htlock.o $(BUILDIR)/clh.o $(BUILDIR)/concurrent_hash_map.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	rm -f $(BINS)
