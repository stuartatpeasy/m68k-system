# Makefile for the m68k firmware
#
# Stuart Wallace <stuartw@atom.net>, 2011-2012.
#
# TODO: remove dependency on libc-sw.  This would permit the removal of the
#       "--start-group ... --end-group" ugliness in the firmware link step.
#
include global.mk

FLASHTOOL=/usr/local/bin/flashtool
DFU=/usr/local/bin/dfu
FLASHPROGDEV=/dev/ttyS1

CSOURCES=dfu.c ds17485.c duart.c main.c \
		 device/device.c \
		 device/block/ata/ata.c device/block/partition/partition.c \
		 cpu/exceptions.c cpu/utilities.c \
		 fs/mount.c fs/vfs.c \
		 memory/buddy.c memory/heap.c memory/kmalloc.c memory/ramdetect.c memory/slab.c \
		 sched/sched.c

ASMSOURCES=cpu/syscall.S

OBJLIBS=libc.a libkutil.a libmonitor.a

#
# FIXME: _mulsi3 is used somewhere; find where.
#
LDFLAGS=-g -T rom.ld -nostdlib -static -L$(BASEDIR)/m68000 -L.

LIBS=-lgcc -lc -lkutil -lmonitor

LDSCRIPT=rom.ld

VTABLE=startup.o
OBJECTS=$(CSOURCES:.c=.o) $(ASMSOURCES:.S=.o)

all: $(APPNAME)

.c.o:
	$(CC) $(CFLAGS) -o$@ $<

.S.o:
	$(CC) $(CFLAGS) -o$@ $<

$(APPNAME): $(OBJECTS) $(OBJLIBS) $(VTABLE)
	$(LD) $(OBJECTS) -o$(APPNAME) $(LDFLAGS) --start-group $(LIBS) --end-group
	$(OBJCOPY) -j .text -j .data -j .bss -O binary $(APPNAME) $(APPNAME).rom
	$(OBJCOPY) -i 2 -b 0 -I binary -O binary $(APPNAME).rom $(APPNAME).E.rom
	$(OBJCOPY) -i 2 -b 1 -I binary -O binary $(APPNAME).rom $(APPNAME).O.rom
	chmod -f -x $(APPNAME).E.rom $(APPNAME).O.rom $(APPNAME).rom || true

evenrom:
	$(FLASHTOOL) --program --infile $(APPNAME).E.rom --verify --device $(FLASHPROGDEV)

oddrom:
	$(FLASHTOOL) --program --infile $(APPNAME).O.rom --verify --device $(FLASHPROGDEV)

roms:
	$(shell read -p 'Insert odd ROM and press <Enter>: ' dummy)
	$(FLASHTOOL) --program --infile $(APPNAME).E.rom --verify
	$(shell read -p 'Insert even ROM and press <Enter>: ' dummy)
	$(FLASHTOOL) --program --infile $(APPNAME).O.rom --verify

dfu:
	$(DFU) --program --infile $(APPNAME).rom

clean:
	rm -f $(APPNAME) $(OBJECTS) $(VTABLE) $(APPNAME).E.rom $(APPNAME).O.rom $(APPNAME).rom
	$(MAKE) $(MFLAGS) -Cklibc clean
	$(MAKE) $(MFLAGS) -Ckutil clean
	$(MAKE) $(MFLAGS) -Cmonitor clean

libc.a:
	$(MAKE) $(MFLAGS) -Cklibc

libkutil.a:
	$(MAKE) $(MFLAGS) -Ckutil

libmonitor.a:
	$(MAKE) $(MFLAGS) -Cmonitor

#
# Dependencies below this point
#

main.o: main.c ds17485.o duart.o device/device.o sched/sched.o
dfu.o: dfu.c dfu.h
ds17485.o: ds17485.c ds17485.h
duart.o: duart.c duart.h
startup.o: startup.S

cpu/exceptions.o: cpu/exceptions.c cpu/exceptions.h cpu/utilities.o cpu/exceptionstackframes.h \
				  include/types.h kutil/kutil.h
cpu/utilities.o: cpu/utilities.c cpu/utilities.h cpu/exceptionstackframes.h include/types.h \
				 kutil/kutil.h

device/device.o: device/device.c device/device.h kutil/kutil.h device/block/ata/ata.h

device/block/ata/ata.o: device/block/ata/ata.c device/block/ata/ata.h \
                        device/block/ata/ata-internal.h \
						include/byteorder.h include/types.h device/device.h \
						device/devctl.h
device/block/partition/partition.o: device/block/partition/partition.c \
									device/block/partition/partition.h \
									device/block/partition/mbr.h include/defs.h include/types.h \
									device/device.h kutil/kutil.h asm/bswap.h device/devctl.h

fs/ext2/ext2.o: fs/ext2/ext2.c fs/ext2/ext2.h include/defs.h klibc/errno.h kutil/kutil.h \
                memory/kmalloc.h
fs/mount.o: fs/mount.c fs/mount.h include/types.h device/device.h include/defs.h
fs/vfs.o: fs/vfs.c fs/vfs.h fs/mount.h include/types.h include/defs.h

include/byteorder.h: asm/bswap.h

memory/buddy.o: memory/buddy.c memory/buddy.h include/types.h \
				kutil/kutil.h libkutil.a
memory/heap.o: memory/heap.c memory/heap.h
memory/kmalloc.o: memory/kmalloc.c memory/kmalloc.h memory/memorymap.h
memory/ramdetect.o: memory/memorymap.h memory/ramdetect.c memory/ramdetect.h include/defs.h \
					include/types.h
memory/slab.o: memory/slab.c memory/slab.h include/defs.h include/types.h

sched/sched.o: sched/sched.c sched/sched.h include/types.h

