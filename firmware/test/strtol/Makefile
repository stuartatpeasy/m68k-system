APPNAME=strtoltest
SOURCES=main.c ../../libc-sw/strtol.c

CC=gcc
CFLAGS=-c -Wall -O3 -I../.. -I../../libc-sw

OBJECTS=$(SOURCES:.c=.o)

all: $(APPNAME)

$(APPNAME): $(OBJECTS)
	$(CC) $(OBJECTS) -o$(APPNAME)

.c.o:
	$(CC) $(CFLAGS) $< -o$@

clean:
	rm -f $(APPNAME) $(OBJECTS)

#main.o: main.c ../../libc-sw/strtol.c ../libc-sw/include/stdlib.h
main.o: main.c ../../libc-sw/strtol.c
../../libc-sw/strtol.o: ../../libc-sw/strtol.c ../../libc-sw/include/stdlib.h
