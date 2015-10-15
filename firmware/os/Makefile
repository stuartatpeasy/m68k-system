# Makefile for the as-yet-unnamed MC68010 operating system
#
# Stuart Wallace <stuartw@atom.net>, 2011-2015.
#
#
include global.mk

FLASHTOOL=/usr/local/bin/flashtool
DFU=/usr/local/bin/dfu
FLASHPROGDEV=/dev/ttyS1

CSOURCES=main.c \
		 cpu/cpu.c cpu/$(CPU)/$(CPU).c \
		 device/nvram.c device/device.c device/ds17485.c device/encx24j600.c device/mc68681.c \
		 device/block/ata/ata.c device/block/partition/partition.c \
		 fs/file.c fs/mount.c fs/vfs.c fs/ext2/ext2.c fs/fat/fat.c \
		 kernel/elf.c kernel/ksym.c kernel/sched.c kernel/syscalls.c \
		 memory/buddy.c memory/extents.c memory/heap.c memory/kmalloc.c memory/slab.c

ASMSOURCES=cpu/$(CPU)/syscall.S cpu/$(CPU)/irq.S cpu/$(CPU)/startup.S

LDSCRIPT=platform/$(PLATFORM)/kernel.ld

OBJLIBS=libc.a libkutil.a libmonitor.a

TOOLS=tools/bin/makemap

MAPFILE=$(APPNAME).map

LDFLAGS=-g -T $(LDSCRIPT) -nostdlib -static -L$(BASEDIR)/m68000 -L.

LIBS=-lgcc -lc -lkutil -lmonitor -lplatform

VTABLE=cpu/$(CPU)/startup.o
OBJECTS=$(CSOURCES:.c=.o) $(ASMSOURCES:.S=.o)

all: $(APPNAME)

.c.o:
	$(CC) $(CFLAGS) -o$@ $<

.S.o:
	$(CC) $(CFLAGS) -o$@ $<

$(APPNAME): $(OBJECTS) $(OBJLIBS) $(VTABLE) $(TOOLS)
	$(MAKE) $(MFLAGS) -Cplatform/$(PLATFORM)
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
	$(MAKE) $(MFLAGS) -Cplatform/$(PLATFORM) clean
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
	main.c device/devctl.h device/device.h device/ds17485.h fs/vfs.h include/defs.h klibc/stdio.h \
	klibc/strings.h kutil/kutil.h memory/extents.o memory/kmalloc.h memory/slab.h \
	monitor/monitor.h device/device.o kernel/sched.h

cpu/cpu.o: \
	cpu/cpu.c cpu/cpu.h cpu/arch_specific.h include/types.h

cpu/$(CPU)/$(CPU).o: \
	cpu/cpu.h cpu/$(CPU)/$(CPU).h cpu/$(CPU)/$(CPU).c include/defs.h include/types.h

startup.o: \
	cpu/$(CPU)/startup.S memory/memorymap.h

device/nvram.o: \
	device/nvram.c device/nvram.h device/ds17485.h include/defs.h include/error.h include/types.h \
	kutil/kutil.h

device/device.o: \
	device/device.c device/devctl.h device/device.h kutil/kutil.h device/block/ata/ata.h \
	device/block/partition/partition.h include/types.h klibc/errno.h

device/ds17485.o: \
	device/ds17485.c device/ds17485.h include/types.h kutil/kutil.h

device/encx24j600.o: \
	device/encx24j600.c device/encx24j600.h cpu/cpu.h device/device.h include/byteorder.h \
	include/defs.h include/error.h include/types.h

device/mc68681.o: \
	device/mc68681.c device/mc68681.h include/defs.h include/error.h include/types.h

device/block/ata/ata.o: \
	device/block/ata/ata.c device/block/ata/ata.h device/block/block.h device/devctl.h \
	device/device.h include/byteorder.h include/defs.h include/mbr.h include/types.h \
	klibc/stdio.h klibc/string.h klibc/strings.h

device/block/partition/partition.o: \
	device/block/partition/partition.c device/block/partition/partition.h \
	device/device.h include/byteorder.h include/defs.h include/mbr.h include/types.h \
	klibc/stdio.h kutil/kutil.h cpu/cpu.h device/devctl.h

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
	fs/vfs.c fs/vfs.h fs/mount.h device/nvram.h device/devctl.h device/device.h include/defs.h \
	include/types.h klibc/stdio.h

include/byteorder.h: \
	cpu/cpu.h

kernel/elf.o: \
	kernel/elf.c include/byteorder.h include/defs.h include/elf.h include/types.h klibc/errno.h \
	klibc/stdio.h klibc/string.h klibc/strings.h

kernel/ksym.o: \
	kernel/ksym.c kernel/ksym.h include/defs.h include/types.h

kernel/sched.o: \
	kernel/sched.c kernel/sched.h cpu/cpu.h include/defs.h include/types.h klibc/strings.h

kernel/syscalls.o: \
	kernel/syscalls.c kernel/syscall.h

memory/buddy.o: \
	memory/buddy.c memory/buddy.h include/types.h klibc/stdio.h kutil/kutil.h libkutil.a

memory/extents.o: \
	memory/extents.c memory/extents.h include/defs.h include/types.h klibc/strings.h

memory/heap.o: \
	memory/heap.c memory/heap.h include/types.h

memory/kmalloc.o: \
	memory/kmalloc.c memory/kmalloc.h include/types.h memory/memorymap.h

memory/slab.o: \
	memory/slab.c memory/slab.h include/defs.h include/types.h klibc/stdio.h
