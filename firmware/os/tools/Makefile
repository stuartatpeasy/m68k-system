# Makefile for host tools
#
# Part of the as-yet-unnamed MC68010 operating system
#
#
# Stuart Wallace <stuartw@atom.net>, September 2015.
#

include ../global.mk

BINDIR=bin

TARGET=$(BINDIR)/makemap
CSOURCES=makemap.c

CFLAGS=-c -O3 -Wall -pedantic

OBJECTS=$(CSOURCES:.c=.o)

all: $(TARGET)

.c.o:
	$(HOSTCC) $(CFLAGS) -o$@ $<

$(TARGET): $(OBJECTS)
	$(HOSTCC) $(OBJECTS) -o$(TARGET)

clean:
	rm -f $(TARGET) $(OBJECTS)

#
# Dependencies below this point
#

makemap.o: \
	makemap.c

