ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS  = $(BINDIR)/lb-bst 
PROF = $(ROOT)/src

.PHONY:	all clean

all:	main

measurements.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/measurements.o $(PROF)/measurements.c

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

bst_bronson_java.o: 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/bst_bronson_java.o bst_bronson_java.c

bst_bronson_java_test.o: bst_bronson_java.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/bst_bronson_java_test.o bst_bronson_java_test.c

bst_bronson_java_test2.o: bst_bronson_java.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/bst_bronson_java_test2.o bst_bronson_java_test2.c

bst_lock_bronson_test_scalability.o: bst_bronson_java.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/bst_bronson_java_test_scalability.o bst_bronson_java_test_scalability.c

main: measurements.o ssalloc.o  bst_bronson_java.o bst_bronson_java_test_scalability.o
	$(CC) $(CFLAGS)  $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/bst_bronson_java.o $(BUILDIR)/bst_bronson_java_test_scalability.o -o $(BINS) $(LDFLAGS)

test2: measurements.o ssalloc.o  bst_bronson_java.o bst_bronson_java_test2.o
	$(CC) $(CFLAGS)  $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/bst_bronson_java.o $(BUILDIR)/bst_bronson_java_test2.o -o $(BINS) $(LDFLAGS)

test1: measurements.o ssalloc.o  bst_bronson_java.o bst_bronson_java_test.o
	$(CC) $(CFLAGS)  $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/bst_bronson_java.o $(BUILDIR)/bst_bronson_java_test.o -o $(BINS) $(LDFLAGS)
clean:
	rm -f $(BINS)
