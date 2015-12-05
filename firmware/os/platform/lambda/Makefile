SRC_ROOT=../..
KERN_ROOT=$(SRC_ROOT)/kernel
KERN_INC=$(KERN_ROOT)/include

include $(SRC_ROOT)/global.mk

LIBNAME=$(SRC_ROOT)/libplatform.a

CSOURCES=device.c dfu.c lambda.c

ASMSOURCES=startup.S

OBJECTS=$(CSOURCES:.c=.o) $(ASMSOURCES:.S=.o)

STDDEPS=$(KERN_ROOT)/platform.h $(KERN_INC)/defs.h $(KERN_INC)/error.h $(KERN_INC)/types.h

all: $(LIBNAME)

.c.o:
	$(CC) $(CFLAGS) -o$@ $<

.S.o:
	$(CC) $(CFLAGS) -o$@ $<

$(LIBNAME): $(OBJECTS)
	$(AR) rcs $(LIBNAME) $(OBJECTS)

clean:
	rm -f $(LIBNAME) $(OBJECTS)

device.o: \
	$(STDDEPS) device.c device.h $(SRC_ROOT)/kernel/device/ata.h $(SRC_ROOT)/kernel/device/auto.h \
	$(SRC_ROOT)/driver/ds17485.h $(SRC_ROOT)/driver/mc68681.h $(KERN_ROOT)/memory/kmalloc.h \
	$(KERN_ROOT)/util/kutil.h $(SRC_ROOT)/klibc/stdio.h $(SRC_ROOT)/klibc/string.h

dfu.o: \
	$(STDDEPS) dfu.c dfu.h lambda.h $(KERN_ROOT)/platform.h $(KERN_ROOT)/util/kutil.h \
	$(KERN_ROOT)/memory/kmalloc.h

lambda.o: \
	$(STDDEPS) lambda.c lambda.h

startup.o: \
	startup.S
