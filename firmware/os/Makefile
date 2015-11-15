# Makefile for the as-yet-unnamed MC68010 operating system
#
# Stuart Wallace <stuartw@atom.net>, 2011-2015.
#
#
include global.mk

FLASHTOOL=/usr/local/bin/flashtool
DFU=/usr/local/bin/dfu
FLASHPROGDEV=/dev/ttyS1

CSOURCES:=	main.c \
			cpu/cpu.c cpu/$(CPU)/$(CPU).c \
			device/ata.c device/auto.c device/device.c device/ds17485.c device/encx24j600.c \
			device/mc68681.c device/nvram.c device/partition.c \
			kernel/fs/file.c kernel/fs/mount.c kernel/fs/vfs.c kernel/fs/ext2/ext2.c \
			kernel/fs/fat/fat.c \
			kernel/boot.c kernel/elf.c kernel/ksym.c kernel/process.c kernel/sched.c \
			kernel/semaphore.c kernel/syscall.c kernel/user.c \
			kernel/net/arp.c kernel/net/ethernet.c kernel/net/ipv4.c \
			kernel/memory/buddy.c kernel/memory/extents.c kernel/memory/heap.c\
			kernel/memory/kmalloc.c kernel/memory/slab.c

ASMSOURCES:=cpu/$(CPU)/syscall.S cpu/$(CPU)/irq.S cpu/$(CPU)/process.S cpu/$(CPU)/startup.S

LDSCRIPT:=platform/$(PLATFORM)/kernel.ld

OBJLIBS:=libc.a libkutil.a libmonitor.a

TOOLS:=tools/bin/makemap

LDFLAGS:=-g -T $(LDSCRIPT) -nostdlib -static -L$(BASEDIR)/m68000 -L.

LIBS:=-lgcc -lc -lkutil -lmonitor -lplatform

VTABLE:=cpu/$(CPU)/startup.o
OBJECTS:=$(CSOURCES:.c=.o) $(ASMSOURCES:.S=.o)

all: $(APPNAME)

.c.o:
	$(CC) $(CFLAGS) -o$@ $<

.S.o:
	$(CC) $(CFLAGS) -o$@ $<

$(APPNAME): $(OBJECTS) $(OBJLIBS) $(VTABLE) $(TOOLS)
	$(MAKE) $(MFLAGS) -Cplatform/$(PLATFORM)
	$(LD) $(OBJECTS) -o$(APPNAME) $(LDFLAGS) --start-group $(LIBS) --end-group
	$(OBJCOPY) -j .text -j .rodata -j .data -O binary $(APPNAME) $(APPNAME).rom
	truncate --size %4 $(APPNAME).rom
	$(NM) $(APPNAME) | tools/bin/makemap >>$(APPNAME).rom
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
	rm -f $(APPNAME) $(OBJECTS) $(VTABLE) $(APPNAME).?.rom $(APPNAME).rom
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
	main.c device/ds17485.h kernel/fs/vfs.h include/defs.h klibc/stdio.h klibc/strings.h \
	kutil/kutil.h kernel/memory/extents.o kernel/memory/kmalloc.h kernel/memory/slab.h \
	monitor/monitor.h device/device.o kernel/sched.h

cpu/cpu.o: \
	cpu/cpu.c cpu/cpu.h cpu/arch_specific.h include/types.h

cpu/$(CPU)/$(CPU).o: \
	cpu/cpu.h cpu/$(CPU)/$(CPU).h cpu/$(CPU)/$(CPU).c include/defs.h include/types.h

cpu/$(CPU)/process.o: \
	cpu/$(CPU)/process.S

cpu/$(CPU)/startup.o: \
	cpu/$(CPU)/startup.S kernel/memory/memorymap.h

device/ata.o: \
	device/ata.c device/ata.h device/block.h device/devctl.h device/device.h include/byteorder.h \
	include/defs.h include/mbr.h include/types.h klibc/stdio.h klibc/string.h klibc/strings.h

device/auto.o: \
	device/auto.c device/auto.h include/defs.h include/types.h device/device.h

device/device.o: \
	device/device.c device/devctl.h device/device.h kutil/kutil.h device/ata.h device/partition.h \
	include/types.h klibc/errno.h

device/ds17485.o: \
	device/ds17485.c device/ds17485.h include/types.h kutil/kutil.h

device/encx24j600.o: \
	device/encx24j600.c device/encx24j600.h cpu/cpu.h device/device.h include/byteorder.h \
	include/defs.h include/error.h include/types.h

device/mc68681.o: \
	device/mc68681.c device/mc68681.h include/defs.h include/error.h include/types.h

device/nvram.o: \
	device/nvram.c device/nvram.h device/ds17485.h include/defs.h include/error.h include/types.h \
	kutil/kutil.h

device/partition.o: \
	device/partition.c device/partition.h device/device.h include/byteorder.h include/defs.h \
	include/mbr.h include/types.h klibc/stdio.h kutil/kutil.h cpu/cpu.h device/devctl.h

kernel/fs/ext2/ext2.o: \
	kernel/fs/ext2/ext2.c kernel/fs/ext2/ext2.h include/defs.h klibc/errno.h klibc/stdio.h \
	klibc/string.h kutil/kutil.h kernel/memory/kmalloc.h

kernel/fs/fat/fat.o: \
	kernel/fs/fat/fat.c kernel/fs/fat/fat.h kernel/fs/vfs.h include/byteorder.h include/defs.h \
	include/error.h include/types.h klibc/strings.h kutil/kutil.h

kernel/fs/file.o: \
	kernel/fs/file.c kernel/fs/file.h kernel/fs/vfs.h include/defs.h include/types.h

kernel/fs/mount.o: \
	kernel/fs/mount.c kernel/fs/mount.h include/defs.h include/types.h device/device.h \
	kernel/fs/vfs.h include/defs.h klibc/errno.h kutil/kutil.h kernel/memory/kmalloc.h

kernel/fs/vfs.o: \
	kernel/fs/vfs.c kernel/fs/vfs.h kernel/fs/mount.h device/nvram.h device/devctl.h \
	device/device.h include/defs.h include/types.h klibc/stdio.h

include/byteorder.h: \
	cpu/cpu.h

kernel/boot.o: \
	kernel/boot.c kernel/boot.h include/defs.h include/types.h device/devctl.h device/device.h \
	klibc/stdio.h platform/platform.h

kernel/elf.o: \
	kernel/elf.c include/byteorder.h include/defs.h include/elf.h include/types.h klibc/errno.h \
	klibc/stdio.h klibc/string.h klibc/strings.h

kernel/ksym.o: \
	kernel/ksym.c kernel/ksym.h include/defs.h include/types.h

kernel/process.o: \
	kernel/process.c kernel/process.h include/defs.h include/types.h

kernel/sched.o: \
	kernel/sched.c kernel/sched.h cpu/cpu.h include/defs.h include/types.h kernel/user.h \
	klibc/strings.h

kernel/semaphore.o: \
	kernel/semaphore.c kernel/semaphore.h include/defs.h include/types.h cpu/cpu.h

kernel/syscall.o: \
	kernel/syscall.c kernel/syscall.h kernel/syscalls.h

kernel/user.o: \
	kernel/user.c kernel/user.h include/defs.h include/types.h

kernel/net/arp.o: \
	kernel/net/arp.c kernel/net/arp.h include/byteorder.h include/defs.h include/error.h \
	include/types.h kernel/net/ethernet.h

kernel/net/ethernet.o: \
	kernel/net/ethernet.c kernel/net/ethernet.h include/byteorder.h include/defs.h \
	include/types.h kernel/net/ipv4.h

kernel/net/ipv4.o: \
	kernel/net/ipv4.c kernel/net/ipv4.h include/byteorder.h include/defs.h include/types.h \
	kernel/net/ethernet.h

kernel/memory/buddy.o: \
	kernel/memory/buddy.c kernel/memory/buddy.h include/types.h klibc/stdio.h kutil/kutil.h \
	libkutil.a

kernel/memory/extents.o: \
	kernel/memory/extents.c kernel/memory/extents.h include/defs.h include/types.h klibc/strings.h

kernel/memory/heap.o: \
	kernel/memory/heap.c kernel/memory/heap.h include/types.h klibc/stdio.h

kernel/memory/kmalloc.o: \
	kernel/memory/kmalloc.c kernel/memory/kmalloc.h include/types.h kernel/memory/memorymap.h

kernel/memory/slab.o: \
	kernel/memory/slab.c kernel/memory/slab.h include/defs.h include/types.h klibc/stdio.h
