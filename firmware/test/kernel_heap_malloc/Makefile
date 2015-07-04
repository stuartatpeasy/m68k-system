APPNAME=malloctest
SOURCES=main.c ../../os/malloc.c

CC=gcc
CFLAGS=-c -Wall -O3 -I../../os

OBJECTS=$(SOURCES:.c=.o)

all: $(APPNAME)

$(APPNAME): $(OBJECTS)
	$(CC) $(OBJECTS) -o$(APPNAME)

.c.o:
	$(CC) $(CFLAGS) $< -o$@

clean:
	rm -f $(APPNAME) $(OBJECTS)

main.o: main.c ../../os/malloc.c ../../os/malloc.h
../../os/malloc.o: ../../os/malloc.c ../../os/malloc.h
