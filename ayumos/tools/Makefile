# Makefile for host tools
#
# Part of ayumos
#
#
# Stuart Wallace <stuartw@atom.net>, September 2015.
#

include ../global.mk

BINDIR=bin

TARGET=$(BINDIR)/makemap
CSOURCES=makemap.c

CFLAGS=-c -O3 -Wall -I..

OBJECTS=$(CSOURCES:.c=.o)

.PHONY: $(BINDIR)

all: $(BINDIR)/makemap $(BINDIR)/mkromfs

.c.o:
	echo "(CC) $<"
	$(CC) -c $(CFLAGS) -o$@ $<

$(BINDIR)/makemap: makemap.o
	mkdir -p $(BINDIR)
	$(CC) -o$@ $<

$(BINDIR)/mkromfs: mkromfs.o
	mkdir -p $(BINDIR)
	$(CC) -o$@ $<

makemap_clean:
	rm -f makemap.o $(BINDIR)/makemap

mkromfs_clean:
	rm -f mkromfs.o $(BINDIR)/mkromfs

clean: makemap_clean mkromfs_clean

#
# Dependencies below this point
#

makemap.o: \
	makemap.c

mkromfs.o: \
	mkromfs.c

