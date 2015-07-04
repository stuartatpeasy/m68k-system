APPNAME=strtoultest
SOURCES=main.c ../../libc-sw/strtoul.c

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
main.o: main.c ../../libc-sw/strtoul.c
../../libc-sw/strtoul.o: ../../libc-sw/strtoul.c ../../libc-sw/include/stdlib.h
