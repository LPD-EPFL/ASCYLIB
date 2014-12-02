ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS  = $(BINDIR)/lb-ht_tbb
PROF = $(ROOT)/src

CC = g++
LDFLAGS += -L/home/trigonak/code/tbb42_20140122oss/build/linux_intel64_gcc_cc4.8_libc2.17_kernel3.11.0_release -ltbb
TEST_FILE = test_simple.cc

.PHONY:	all clean

all:	main

measurements.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/measurements.o $(PROF)/measurements.c

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

test.o: $(TEST_FILE)
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE) -I/home/trigonak/code/tbb42_20140122oss/include

main: 	test.o ssalloc.o measurements.o
	$(CC) $(CFLAGS) $(BUILDIR)/ssalloc.o $(BUILDIR)/measurements.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS) 

clean:
	rm -f $(BINS)
