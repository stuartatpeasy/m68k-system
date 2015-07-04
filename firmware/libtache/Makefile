APPNAME=libtache.a
PLATFORM=m68000

CC=m68k-coff-gcc
AR=m68k-coff-ar
AS=m68k-coff-as
LD=m68k-coff-ld

BASEDIR=/opt/m68k/lib/gcc/m68k-coff/3.4.2

AFLAGS=-$(PLATFORM)
CFLAGS=-$(PLATFORM) -Wall -O2 -c
LDFLAGS=-nostdlib -static -L$(BASEDIR)/m68000

ASMSOURCES=malloc.S free.S

OBJECTS=$(ASMSOURCES:.S=.o)

all: $(APPNAME)

.S.o:
	$(AS) $(AFLAGS) -o$@ $<

$(APPNAME): $(OBJECTS)
	$(AR) rcs $(APPNAME) $(OBJECTS)

test:
	$(CC) $(CFLAGS) -otest.o test.c
	$(LD) test.o -otest -L. -ltache

clean:
	rm -f $(APPNAME) $(OBJECTS) test test.o

free.o: free.S tache.h
malloc.o: malloc.S tache.h

