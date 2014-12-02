ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS  = $(BINDIR)/lb-bst_bronson
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

bst_bronson_java_test_scalability.o: bst_bronson_java.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/bst_bronson_java_test_scalability.o bst_bronson_java_test_scalability.c

igor_test.o: bst_bronson_java.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/igor_test.o igor_test.c

oana_test.o: bst_bronson_java.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/oana_test.o oana_test.c

#main: measurements.o ssalloc.o  bst_bronson_java.o bst_bronson_java_test_scalability.o
#	$(CC) $(CFLAGS)  $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/bst_bronson_java.o $(BUILDIR)/bst_bronson_java_test_scalability.o -o $(BINS) $(LDFLAGS)

test2: measurements.o ssalloc.o  bst_bronson_java.o bst_bronson_java_test2.o
	$(CC) $(CFLAGS)  $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/bst_bronson_java.o $(BUILDIR)/bst_bronson_java_test2.o -o $(BINS) $(LDFLAGS)

test1: measurements.o ssalloc.o  bst_bronson_java.o bst_bronson_java_test.o
	$(CC) $(CFLAGS)  $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/bst_bronson_java.o $(BUILDIR)/bst_bronson_java_test.o -o $(BINS) $(LDFLAGS)

igor: measurements.o ssalloc.o  bst_bronson_java.o igor_test.o
	$(CC) $(CFLAGS)  $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/bst_bronson_java.o $(BUILDIR)/igor_test.o -o $(BINS) $(LDFLAGS)

oana: measurements.o ssalloc.o  bst_bronson_java.o oana_test.o
	$(CC) $(CFLAGS)  $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/bst_bronson_java.o $(BUILDIR)/oana_test.o -o $(BINS) $(LDFLAGS)

test.o: bst_bronson_java.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: measurements.o ssalloc.o  bst_bronson_java.o test.o 
	$(CC) $(CFLAGS) $(BUILDIR)/measurements.o $(BUILDIR)/ssalloc.o $(BUILDIR)/bst_bronson_java.o  $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	rm -f $(BINS)
