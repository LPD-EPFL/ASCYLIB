VERSION = TCMALLOC_NO_UPDATE

CC = g++ -O3 -std=gnu++0x -g -w

ifeq "$(VERSION)" "MALLOC_UPDATE"
CFLAGS = -lpthread -lrt -DUPDATEVAL
else
ifeq "$(VERSION)" "TCMALLOC_UPDATE"
CFLAGS = -lpthread -lrt /usr/lib64/libtcmalloc.so.4 -DUPDATEVAL
else
ifeq "$(VERSION)" "TCMALLOC_NO_UPDATE"
CFLAGS = -lpthread -lrt /usr/lib64/libtcmalloc.so.4
else
CFLAGS = -lpthread -lrt
endif
endif
endif

wfrbt: wfrbt.o
	$(CC) $(CFLAGS) wfrbt.o -o wfrbt

wfrbt.o: wfrbt.h wfrbt.cpp
	$(CC) $(CFLAGS) -c wfrbt.cpp

clean:
	rm -f wfrbt
	rm -f wfrbt.o
