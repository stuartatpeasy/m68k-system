# Makefile for the as-yet-unnamed MC68010 operating system
#
# Stuart Wallace <stuartw@atom.net>, 2011-2015.
#
#
include global.mk

FLASHTOOL=/usr/local/bin/flashtool
DFU=/usr/local/bin/dfu
FLASHPROGDEV=/dev/ttyS1

CSOURCES=dfu.c main.c \
		 device/bbram.c device/device.c device/ds17485.c device/duart.c device/encx24j600.c \
		 device/expansion.c device/led.c device/block/ata/ata.c device/block/partition/partition.c \
		 cpu/exceptions.c cpu/utilities.c \
		 fs/file.c fs/mount.c fs/vfs.c fs/ext2/ext2.c fs/fat/fat.c \
		 kernel/elf.c kernel/ksym.c kernel/syscalls.c \
		 memory/buddy.c memory/heap.c memory/kmalloc.c memory/ramdetect.c memory/slab.c \
		 sched/sched.c

ASMSOURCES=kernel/syscall.S

OBJLIBS=libc.a libkutil.a libmonitor.a

TOOLS=tools/bin/makemap

MAPFILE=$(APPNAME).map

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

$(APPNAME): $(OBJECTS) $(OBJLIBS) $(VTABLE) $(TOOLS)
	$(LD) $(OBJECTS) -o$(APPNAME) $(LDFLAGS) --start-group $(LIBS) --end-group
	$(NM) $(APPNAME) > $(MAPFILE)
	$(OBJCOPY) -j .text -j .rodata -j .data -O binary $(APPNAME) $(APPNAME).rom
	truncate --size %4 $(APPNAME).rom
	tools/bin/makemap <$(MAPFILE) >>$(APPNAME).rom
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
	$(DFU) --program --baud 115200 --infile $(APPNAME).rom

clean:
	rm -f $(APPNAME) $(OBJECTS) $(VTABLE) $(APPNAME).?.rom $(APPNAME).rom $(MAPFILE)
	$(MAKE) $(MFLAGS) -Cklibc clean
	$(MAKE) $(MFLAGS) -Ckutil clean
	$(MAKE) $(MFLAGS) -Cmonitor clean
	$(MAKE) $(MFLAGS) -Ctools clean

libc.a:
	$(MAKE) $(MFLAGS) -Cklibc

libkutil.a:
	$(MAKE) $(MFLAGS) -Ckutil

libmonitor.a:
	$(MAKE) $(MFLAGS) -Cmonitor

$(TOOLS):
	mkdir -p tools/bin
	$(MAKE) $(MFLAGS) -Ctools


#
# Dependencies below this point
#

main.o: \
	main.c cpu/utilities.h device/devctl.h device/device.h device/expansion.h device/led.h \
	device/ds17485.h device/duart.h fs/vfs.h include/defs.h klibc/stdio.h klibc/strings.h \
	kutil/kutil.h memory/kmalloc.h memory/ramdetect.h memory/slab.h monitor/monitor.h \
	device/duart.o device/device.o sched/sched.h

dfu.o: \
	dfu.c dfu.h asm/reset.h cpu/exceptions.h device/duart.h include/types.h include/error.h \
	kutil/kutil.h memory/kmalloc.h memory/memorymap.h klibc/string.h

startup.o: \
	startup.S memory/memorymap.h

cpu/exceptions.o: \
    asm/rte.h cpu/exceptions.c cpu/exceptions.h cpu/utilities.o cpu/exceptionstackframes.h \
	include/types.h kutil/kutil.h klibc/stdio.h sched/sched.h

cpu/utilities.o: \
    cpu/utilities.c cpu/utilities.h cpu/exceptionstackframes.h include/types.h \
	klibc/stdio.h kutil/kutil.h

device/bbram.o: \
	device/bbram.c device/bbram.h device/ds17485.h include/defs.h include/error.h include/types.h \
	kutil/kutil.h

device/device.o: \
	device/device.c device/devctl.h device/device.h kutil/kutil.h device/block/ata/ata.h \
	device/block/partition/partition.h include/types.h klibc/errno.h

device/ds17485.o: \
	device/ds17485.c device/ds17485.h include/types.h kutil/kutil.h

device/encx24j600.o: \
	device/encx24j600.c device/encx24j600.h cpu/exceptions.h device/device.h device/expansion.h \
	include/defs.h include/error.h include/types.h

device/duart.o: \
	device/duart.c device/duart.h include/defs.h include/types.h

device/expansion.o: \
	device/expansion.c device/expansion.h cpu/exceptions.h device/duart.h include/defs.h \
	include/types.h klibc/stdio.h

device/led.o: \
	device/led.c device/led.h device/duart.h include/defs.h include/types.h

device/block/ata/ata.o: \
	device/block/ata/ata.c device/block/ata/ata.h device/block/ata/ata-internal.h \
	device/block/block.h device/block/partition/mbr.h device/devctl.h device/device.h \
	include/byteorder.h include/defs.h include/types.h klibc/stdio.h klibc/string.h klibc/strings.h

device/block/partition/partition.o: \
	device/block/partition/partition.c device/block/partition/partition.h \
	device/block/partition/mbr.h device/device.h include/byteorder.h include/defs.h \
	include/types.h klibc/stdio.h kutil/kutil.h asm/bswap.h device/devctl.h

fs/ext2/ext2.o: \
	fs/ext2/ext2.c fs/ext2/ext2.h include/defs.h klibc/errno.h klibc/stdio.h klibc/string.h \
	kutil/kutil.h memory/kmalloc.h

fs/fat/fat.o: \
	fs/fat/fat.c fs/fat/fat.h fs/vfs.h include/byteorder.h include/defs.h include/error.h \
	include/types.h klibc/strings.h kutil/kutil.h

fs/file.o: \
	fs/file.c fs/file.h fs/vfs.h include/defs.h include/types.h

fs/mount.o: \
	fs/mount.c fs/mount.h include/defs.h include/types.h device/device.h fs/vfs.h include/defs.h \
	klibc/errno.h kutil/kutil.h memory/kmalloc.h

fs/vfs.o: \
	fs/vfs.c fs/vfs.h fs/mount.h device/bbram.h device/devctl.h device/device.h include/defs.h \
	include/types.h klibc/stdio.h

include/byteorder.h: \
	asm/bswap.h

kernel/elf.o: \
	kernel/elf.c include/byteorder.h include/defs.h include/elf.h include/types.h klibc/errno.h \
	klibc/stdio.h klibc/string.h klibc/strings.h

kernel/ksym.o: \
	kernel/ksym.c kernel/ksym.h include/defs.h include/types.h

kernel/syscalls.o: \
	kernel/syscalls.c kernel/syscall.h

memory/buddy.o: \
	memory/buddy.c memory/buddy.h include/types.h klibc/stdio.h kutil/kutil.h libkutil.a

memory/heap.o: \
	memory/heap.c memory/heap.h include/types.h

memory/kmalloc.o: \
	memory/kmalloc.c memory/kmalloc.h include/types.h memory/memorymap.h

memory/ramdetect.o: \
	memory/memorymap.h memory/ramdetect.c memory/ramdetect.h include/defs.h include/types.h

memory/slab.o: \
	memory/slab.c memory/slab.h include/defs.h include/types.h klibc/stdio.h

sched/sched.o: \
	sched/sched.c sched/sched.h cpu/exceptions.h cpu/utilities.h device/duart.h include/defs.h \
	kernel/syscall.h include/types.h klibc/strings.h
