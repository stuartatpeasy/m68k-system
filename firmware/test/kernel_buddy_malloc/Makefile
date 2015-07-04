APPNAME=buddy
SOURCES=main.c buddy.c

CC=gcc
CFLAGS=-c -Wall -O3 -DTEST -I../../os -I../../ -g

OBJECTS=$(SOURCES:.c=.o)

all: $(APPNAME)

.c.o:
	$(CC) $(CFLAGS) $< -o$@

$(APPNAME): $(OBJECTS)
	$(CC) $(OBJECTS) -o$(APPNAME)

clean:
	rm -f $(APPNAME) $(OBJECTS)

buddy.o: buddy.c buddy.h ../../os/include/types.h
main.o: main.c buddy.c buddy.h ../../os/include/types.h
