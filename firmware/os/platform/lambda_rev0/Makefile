SRC_ROOT=../..
OS_INCDIR=$(SRC_ROOT)/include

include $(SRC_ROOT)/global.mk

LIBNAME=$(SRC_ROOT)/libplatform.a

CSOURCES=device.c dfu.c lambda.c

OBJECTS=$(CSOURCES:.c=.o)

STDDEPS=../platform.h $(OS_INCDIR)/defs.h $(OS_INCDIR)/error.h $(OS_INCDIR)/types.h

all: $(LIBNAME)

.c.o:
	$(CC) $(CFLAGS) -o$@ $<

$(LIBNAME): $(OBJECTS)
	$(AR) rcs $(LIBNAME) $(OBJECTS)

clean:
	rm -f $(LIBNAME) $(OBJECTS)

device.o: \
	$(STDDEPS) device.c device.h $(SRC_ROOT)/device/ata.h $(SRC_ROOT)/kernel/device/auto.h \
	$(SRC_ROOT)/device/ds17485.h $(SRC_ROOT)/device/mc68681.h $(SRC_ROOT)/kernel/memory/kmalloc.h \
	$(SRC_ROOT)/kernel/util/kutil.h $(SRC_ROOT)/klibc/stdio.h $(SRC_ROOT)/klibc/string.h

dfu.o: \
	$(STDDEPS) dfu.c dfu.h $(SRC_ROOT)/kernel/util/kutil.h $(SRC_ROOT)/kernel/memory/kmalloc.h \
	$(SRC_ROOT)/kernel/memory/memorymap.h

lambda.o: \
	$(STDDEPS) lambda.c lambda.h

