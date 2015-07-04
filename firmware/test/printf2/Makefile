APPNAME=test_printf
SOURCES=test.c printf.c

CC=gcc
CFLAGS=-c -Wall -O3 -DDEBUG -DTARGET_LITTLEENDIAN -I../../os -I../../ -g

OBJECTS=$(SOURCES:.c=.o)

all: $(APPNAME)

.c.o:
	$(CC) $(CFLAGS) $< -o$@

$(APPNAME): $(OBJECTS)
	$(CC) $(OBJECTS) -o$(APPNAME)

clean:
	rm -f $(APPNAME) $(OBJECTS)

test.o: test.c printf.h
printf.o: printf.c printf.h

