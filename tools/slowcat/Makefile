CC=gcc
APPNAME=slowcat
SOURCES=slowcat.c

OBJECTS=$(SOURCES:.c=.o)

CFLAGS=-c -Wall -O3
LDFLAGS=

all: $(APPNAME)

$(APPNAME): $(OBJECTS)

.c.o:
	$(CC) $(CFLAGS) $< -o$@

clean:
	rm -f $(APPNAME) $(OBJECTS)

slowcat.o: slowcat.c

