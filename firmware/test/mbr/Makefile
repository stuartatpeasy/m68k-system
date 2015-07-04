APPNAME=mbr
SOURCES=main.c mbr.c sector_read.c

CC=gcc
CFLAGS=-c -Wall -O3 -DTEST -DTARGET_LITTLEENDIAN -I../../os -I../../ -g

OBJECTS=$(SOURCES:.c=.o)

all: $(APPNAME)

.c.o:
	$(CC) $(CFLAGS) $< -o$@

$(APPNAME): $(OBJECTS)
	$(CC) $(OBJECTS) -o$(APPNAME)

clean:
	rm -f $(APPNAME) $(OBJECTS)

main.o: main.c sector_read.h ../../os/include/defs.h
mbr.o: mbr.c mbr.h sector_read.h ../../os/include/defs.h
sector_read.o: sector_read.c sector_read.h ../../os/include/defs.h
