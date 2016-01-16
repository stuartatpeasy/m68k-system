# Makefile for the as-yet-unnamed MC68010 operating system
#
# Stuart Wallace <stuartw@atom.net>, 2011-2015.
#
#
include global.mk

FLASHTOOL=/usr/local/bin/flashtool
DFU=/usr/local/bin/dfu
FLASHPROGDEV=/dev/ttyS1

CSOURCES:=	cpu/$(CPU)/$(CPU).c \
			driver/ds17485.c driver/encx24j600.c driver/mc68681.c driver/ps2controller.c \
			kernel/device/ata.c kernel/device/auto.c kernel/device/device.c kernel/device/nvram.c \
			kernel/device/partition.c \
			kernel/fs/file.c kernel/fs/mount.c kernel/fs/vfs.c kernel/fs/ext2/ext2.c \
			kernel/fs/fat/fat.c kernel/fs/romfs.c \
			kernel/boot.c kernel/cpu.c kernel/elf.c kernel/entry.c kernel/housekeeper.c \
			kernel/ksym.c kernel/process.c kernel/sched.c kernel/semaphore.c kernel/syscall.c \
			kernel/user.c kernel/net/arp.c kernel/net/ethernet.c kernel/net/ipv4.c \
			kernel/net/net.c kernel/net/udp.c \
			kernel/memory/buddy.c kernel/memory/extents.c kernel/memory/heap.c\
			kernel/memory/kmalloc.c kernel/memory/slab.c

ASMSOURCES:=cpu/$(CPU)/syscall.S cpu/$(CPU)/irq.S cpu/$(CPU)/process.S

LDSCRIPT:=platform/$(PLATFORM)/kernel.ld

OBJLIBS:=libc.a libkutil.a libmonitor.a

TOOLS:=tools/bin/makemap

LDFLAGS:=-g -T $(LDSCRIPT) -nostdlib -static -L$(BASEDIR)/m68000 -L.

LIBS:=-lgcc -lc -lkutil -lmonitor -lplatform

OBJECTS:=$(CSOURCES:.c=.o) $(ASMSOURCES:.S=.o)

all: $(APPNAME)

.c.o:
	$(CC) $(CFLAGS) -o$@ $<

.S.o:
	$(CC) $(CFLAGS) -o$@ $<

$(APPNAME): $(OBJECTS) $(OBJLIBS) $(TOOLS)
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
	rm -f $(APPNAME) $(OBJECTS) $(APPNAME).?.rom $(APPNAME).rom
	$(MAKE) $(MFLAGS) -Cplatform/$(PLATFORM) clean
	$(MAKE) $(MFLAGS) -Cklibc clean
	$(MAKE) $(MFLAGS) -Ckernel/util clean
	$(MAKE) $(MFLAGS) -Cmonitor clean
	$(MAKE) $(MFLAGS) -Ctools clean

libc.a:
	$(MAKE) $(MFLAGS) -Cklibc

libkutil.a:
	$(MAKE) $(MFLAGS) -Ckernel/util

libmonitor.a:
	$(MAKE) $(MFLAGS) -Cmonitor

$(TOOLS):
	mkdir -p tools/bin
	$(MAKE) $(MFLAGS) -Ctools


#
# Dependencies below this point
#

cpu/$(CPU)/$(CPU).o: \
	kernel/cpu.h cpu/$(CPU)/$(CPU).h cpu/$(CPU)/$(CPU).c kernel/include/defs.h \
	kernel/include/types.h

cpu/$(CPU)/process.o: \
	cpu/$(CPU)/process.S

driver/ds17485.o: \
	driver/ds17485.c driver/ds17485.h kernel/include/types.h kernel/util/kutil.h

driver/encx24j600.o: \
	driver/encx24j600.c driver/encx24j600.h kernel/cpu.h kernel/device/device.h \
	kernel/include/byteorder.h kernel/include/defs.h kernel/include/error.h kernel/include/types.h

driver/mc68681.o: \
	driver/mc68681.c driver/mc68681.h kernel/include/defs.h kernel/include/error.h \
	kernel/include/types.h

driver/ps2controller.o: \
	driver/ps2controller.c driver/ps2controller.h kernel/include/defs.h kernel/include/error.h \
	kernel/include/types.h

kernel/boot.o: \
	kernel/boot.c kernel/boot.h kernel/include/defs.h kernel/include/types.h \
	kernel/device/devctl.h kernel/device/device.h klibc/stdio.h kernel/platform.h

kernel/cpu/cpu.o: \
	kernel/cpu/cpu.c kernel/cpu.h cpu/arch_specific.h kernel/include/types.h

kernel/device/ata.o: \
	kernel/device/ata.c kernel/device/ata.h kernel/device/block.h kernel/device/devctl.h \
	kernel/device/device.h kernel/include/byteorder.h kernel/include/defs.h kernel/device/mbr.h \
	kernel/include/types.h klibc/stdio.h klibc/string.h klibc/strings.h

kernel/device/auto.o: \
	kernel/device/auto.c kernel/device/auto.h kernel/include/defs.h kernel/include/types.h \
	kernel/device/device.h

kernel/device/device.o: \
	kernel/device/device.c kernel/device/devctl.h kernel/device/device.h kernel/util/kutil.h \
	kernel/device/ata.h kernel/device/partition.h kernel/include/types.h klibc/errno.h

kernel/device/nvram.o: \
	kernel/device/nvram.c kernel/device/nvram.h driver/ds17485.h kernel/include/defs.h \
	kernel/include/error.h kernel/include/types.h kernel/util/kutil.h

kernel/device/partition.o: \
	kernel/device/partition.c kernel/device/partition.h kernel/device/device.h \
	kernel/include/byteorder.h kernel/include/defs.h kernel/device/mbr.h kernel/include/types.h \
	klibc/stdio.h kernel/util/kutil.h kernel/cpu.h kernel/device/devctl.h

kernel/elf.o: \
	kernel/elf.c kernel/include/byteorder.h kernel/include/defs.h kernel/elf.h \
	kernel/include/types.h klibc/errno.h klibc/stdio.h klibc/string.h klibc/strings.h

kernel/entry.o: \
	kernel/entry.c driver/ds17485.h kernel/fs/vfs.h kernel/include/defs.h klibc/stdio.h \
	klibc/strings.h kernel/util/kutil.h kernel/memory/extents.o kernel/memory/kmalloc.h \
	kernel/memory/slab.h monitor/monitor.h kernel/device/device.o kernel/sched.h

kernel/fs/ext2/ext2.o: \
	kernel/fs/ext2/ext2.c kernel/fs/ext2/ext2.h kernel/include/defs.h klibc/errno.h klibc/stdio.h \
	klibc/string.h kernel/util/kutil.h kernel/memory/kmalloc.h

kernel/fs/fat/fat.o: \
	kernel/fs/fat/fat.c kernel/fs/fat/fat.h kernel/fs/vfs.h kernel/include/byteorder.h \
	kernel/include/defs.h kernel/include/error.h kernel/include/types.h klibc/strings.h \
	kernel/util/kutil.h

kernel/fs/romfs.o: \
	kernel/fs/romfs.c kernel/fs/romfs.h kernel/fs/vfs.h kernel/include/defs.h \
	kernel/include/error.h kernel/include/types.h

kernel/fs/file.o: \
	kernel/fs/file.c kernel/fs/file.h kernel/fs/vfs.h kernel/include/defs.h kernel/include/types.h

kernel/fs/mount.o: \
	kernel/fs/mount.c kernel/fs/mount.h kernel/include/defs.h kernel/include/types.h \
	kernel/device/device.h kernel/fs/vfs.h kernel/include/defs.h klibc/errno.h kernel/util/kutil.h \
	kernel/memory/kmalloc.h

kernel/fs/vfs.o: \
	kernel/fs/vfs.c kernel/fs/vfs.h kernel/fs/mount.h kernel/device/nvram.h \
	kernel/device/devctl.h kernel/device/device.h kernel/include/defs.h kernel/include/types.h \
	klibc/stdio.h

kernel/housekeeper.o: \
	kernel/housekeeper.c kernel/housekeeper.h kernel/include/defs.h kernel/include/types.h

kernel/include/byteorder.h: \
	kernel/cpu.h

kernel/ksym.o: \
	kernel/ksym.c kernel/ksym.h kernel/include/defs.h kernel/include/types.h

kernel/memory/buddy.o: \
	kernel/memory/buddy.c kernel/memory/buddy.h kernel/include/types.h klibc/stdio.h \
	kernel/util/kutil.h libkutil.a

kernel/memory/extents.o: \
	kernel/memory/extents.c kernel/memory/extents.h kernel/include/defs.h kernel/include/types.h \
	klibc/strings.h

kernel/memory/heap.o: \
	kernel/memory/heap.c kernel/memory/heap.h kernel/include/types.h klibc/stdio.h

kernel/memory/kmalloc.o: \
	kernel/memory/kmalloc.c kernel/memory/kmalloc.h kernel/include/types.h

kernel/memory/slab.o: \
	kernel/memory/slab.c kernel/memory/slab.h kernel/include/defs.h kernel/include/types.h \
	klibc/stdio.h

kernel/net/arp.o: \
	kernel/net/arp.c kernel/net/arp.h kernel/include/byteorder.h kernel/include/defs.h \
	kernel/include/error.h kernel/include/types.h kernel/net/ethernet.h

kernel/net/ethernet.o: \
	kernel/net/ethernet.c kernel/net/ethernet.h kernel/include/byteorder.h kernel/include/defs.h \
	kernel/include/types.h kernel/net/ipv4.h

kernel/net/icmp.o: \
	kernel/net/icmp.c kernel/net/icmp.h kernel/include/defs.h kernel/include/types.h

kernel/net/ipv4.o: \
	kernel/net/ipv4.c kernel/net/ipv4.h kernel/include/byteorder.h kernel/include/defs.h \
	kernel/include/types.h kernel/net/ethernet.h

kernel/net/net.o: \
	kernel/net/net.c kernel/net/net.h kernel/include/defs.h kernel/include/types.h \
	kernel/memory/kmalloc.h kernel/net/arp.h

kernel/net/udp.o: \
	kernel/net/udp.c kernel/net/udp.h kernel/include/defs.h kernel/include/types.h

kernel/process.o: \
	kernel/process.c kernel/process.h kernel/include/defs.h kernel/include/types.h

kernel/sched.o: \
	kernel/sched.c kernel/sched.h kernel/cpu.h kernel/include/defs.h kernel/include/types.h \
	kernel/user.h klibc/strings.h

kernel/semaphore.o: \
	kernel/semaphore.c kernel/semaphore.h kernel/include/defs.h kernel/include/types.h kernel/cpu.h

kernel/syscall.o: \
	kernel/syscall.c kernel/syscall.h kernel/syscalls.h

kernel/user.o: \
	kernel/user.c kernel/user.h kernel/include/defs.h kernel/include/types.h
