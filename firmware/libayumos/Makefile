LIBNAME=ayumos
TESTAPP=testapp
PLATFORM=m68000
TARGET_ARCH=m68k

# old-style (gcc 3) config
TARGET_FMT=elf

# new-style (gcc 5) config
TARGET_FMT=linux-elf

TARGET_PREFIX=$(TARGET_ARCH)-$(TARGET_FMT)

CC=$(TARGET_PREFIX)-gcc
AR=$(TARGET_PREFIX)-ar
AS=$(TARGET_PREFIX)-as
LD=$(TARGET_PREFIX)-ld

GCC_VERSION=$(shell $(CC) --version | head -n 1 | sed 's/.* //')

BASEDIR=/opt/m68k
LIBDIR=$(BASEDIR)/lib/gcc/$(TARGET_ARCH)-$(TARGET_FMT)/$(GCC_VERSION)/

AFLAGS=-$(PLATFORM) -Wall -O3 -c -fPIE
CFLAGS=-$(PLATFORM) -Wall -O3 -c -fPIE
LDFLAGS=-nostdlib -static -L$(LIBDIR)/$(PLATFORM)

CSOURCES=

ASMSOURCES=syscalln.S

OBJECTS=$(ASMSOURCES:.S=.o) $(CSOURCES:.c=.o)
LIBFILE=lib$(LIBNAME).a

all: $(LIBFILE) crt0.o testapp

.c.o:
	$(CC) $(CFLAGS) -o$@ $<

.S.o:
	$(CC) $(AFLAGS) -o$@ $<

$(LIBFILE): $(OBJECTS)
	$(AR) rcs $(LIBFILE) $(OBJECTS)

$(TESTAPP): testapp.o
	$(LD) $(LDFLAGS) -nostdlib crt0.o testapp.o -o$(TESTAPP) -L. -l$(LIBNAME) -s

clean:
	rm -f $(LIBFILE) $(OBJECTS) crt0.o $(TESTAPP) $(TESTAPP).o

crt0.o: \
	crt0.S

ayumos.o: \
	ayumos.S 

testapp.o: \
	testapp.c ayumos.h $(LIBFILE)
