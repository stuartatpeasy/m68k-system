APPNAME=vector
SOURCES=bvec.c main.c harness.c

CC=gcc
#CFLAGS=-c -O3 -Wall -DDEBUG -DDEBUG_ALLOC -DTARGET_LITTLEENDIAN -I../../os -I../../ -g
CFLAGS=-c -Wall -DDEBUG -DDEBUG_ALLOC -DTARGET_LITTLEENDIAN -I../../os -I../../ -g

OBJECTS=$(SOURCES:.c=.o)

all: $(APPNAME)

.c.o:
	$(CC) $(CFLAGS) $< -o$@

$(APPNAME): $(OBJECTS)
	$(CC) $(OBJECTS) -o$(APPNAME)

clean:
	rm -f $(APPNAME) $(OBJECTS)

bvec.o: bvec.c bvec.h harness.h
main.o: main.c bvec.h harness.h
harness.o: harness.c harness.h

