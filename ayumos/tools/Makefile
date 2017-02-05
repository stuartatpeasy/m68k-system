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

CFLAGS=-c -O3 -Wall -pedantic

OBJECTS=$(CSOURCES:.c=.o)

all: $(TARGET)

.c.o:
	echo "(HOSTCC) $<"
	$(HOSTCC) $(CFLAGS) -o$@ $<

$(TARGET): $(OBJECTS)
	mkdir -p $(BINDIR)
	echo "(LINK) $(TARGET)"
	$(HOSTCC) -o$(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS)
	$(shell if [ -d "$(BINDIR)" ]; then rmdir -p "$(BINDIR)"; fi)

#
# Dependencies below this point
#

makemap.o: \
	makemap.c

