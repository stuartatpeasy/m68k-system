APPNAME=ext2
SOURCES=ext2.c harness.c main.c unaligned.c

CC=gcc
CFLAGS=-c -Wall -DDEBUG -DTARGET_LITTLEENDIAN -I../../os -I../../ -g

OBJECTS=$(SOURCES:.c=.o)

all: $(APPNAME)

.c.o:
	$(CC) $(CFLAGS) $< -o$@

$(APPNAME): $(OBJECTS)
	$(CC) $(OBJECTS) -o$(APPNAME)

clean:
	rm -f $(APPNAME) $(OBJECTS)

ext2.o: ext2.c ext2.h ext2-defs.h harness.h unaligned.h ../../os/include/defs.h ../../os/include/errno.h
harness.o: harness.c harness.h
main.o: main.c ext2.h ext2-defs.h harness.h
unaligned.o: unaligned.c unaligned.h
