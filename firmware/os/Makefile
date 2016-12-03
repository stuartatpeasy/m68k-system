# Makefile for ayumos
#
# Stuart Wallace <stuartw@atom.net>, 2011-2015.
#
#
include global.mk

FLASHTOOL=/usr/local/bin/flashtool
DFU=/usr/local/bin/dfu
FLASHPROGDEV=/dev/ttyS1

KERNEL_SOURCES := \
	cpu/$(CPU)/$(CPU).c \
	driver/ds17485.c driver/encx24j600.c driver/mc68681.c driver/ps2controller.c \
	kernel/device/ata.c kernel/device/auto.c kernel/device/block.c kernel/device/device.c \
	kernel/device/memconsole.c kernel/device/nvram.c kernel/device/partition.c kernel/fs/file.c \
	kernel/fs/mount.c kernel/fs/vfs.c kernel/fs/ext2/ext2.c kernel/fs/fat/fat.c kernel/fs/romfs.c \
	kernel/boot.c kernel/console.c kernel/cpu.c kernel/elf.c kernel/entry.c kernel/error.c \
	kernel/housekeeper.c kernel/keyboard.c kernel/ksym.c kernel/preempt.c kernel/process.c \
	kernel/sched.c kernel/semaphore.c kernel/syscall.c kernel/tick.c kernel/user.c \
	kernel/net/address.c kernel/net/arp.c kernel/net/dhcp.c kernel/net/ethernet.c \
	kernel/net/icmp.c kernel/net/interface.c kernel/net/ipv4.c kernel/net/net.c \
	kernel/net/packet.c kernel/net/protocol.c kernel/net/raw.c kernel/net/route.c \
	kernel/net/socket.c kernel/net/tcp.c kernel/net/tftp.c kernel/net/udp.c \
	kernel/memory/buddy.c kernel/memory/extents.c kernel/memory/heap.c kernel/memory/kmalloc.c \
	kernel/memory/slab.c \
	kernel/util/bvec.c kernel/util/buffer.c kernel/util/checksum.c kernel/util/dump.c \
	kernel/util/hash.c kernel/util/ktime.c kernel/util/numeric.c kernel/util/string.c

KLIBC_SOURCES := \
	klibc/assert.c klibc/ctype.c klibc/errno.c klibc/printf.c klibc/stdio.c klibc/stdlib.c \
	klibc/string.c klibc/strings.c

MONITOR_SOURCES := \
	monitor/commands.c monitor/disasm.c monitor/env.c monitor/history.c monitor/monitor.o \
	monitor/readline.c monitor/srec.c

ASMSOURCES := cpu/$(CPU)/syscall.S cpu/$(CPU)/irq.S cpu/$(CPU)/process.S

CSOURCES := $(KERNEL_SOURCES) $(KLIBC_SOURCES) $(MONITOR_SOURCES)

SOURCES := $(CSOURCES) $(ASMSOURCES)

LDSCRIPT := platform/$(PLATFORM)/kernel.ld

DEPDIR := .deps
OBJDIR := .obj
$(shell mkdir -p $(DEPDIR) $(OBJDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

OBJECTS := $(patsubst %.c,%.o,$(CSOURCES)) $(patsubst %.S,%.o,$(ASMSOURCES))

COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -c
COMPILE.S = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -c
COMPILE.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c

%.o : %.c $(DEPDIR)/%.d
	$(CC) -o$@ $<

%.o : %.S $(DEPDIR)/%.d
	$(CC) -o$@ $<

$(OBJDIR)/%.o:
	@mkdir -p $(dir $@)

$(DEPDIR)/%.d:
	@mkdir -p $(dir $@)

.PRECIOUS: $(DEPDIR)/%.d
.PRECIOUS: $(OBJDIR)/%.o

TOOLS:=tools/bin/makemap

LDFLAGS:=-g -T $(LDSCRIPT) -nostdlib -static -L$(BASEDIR)/m68000 -L.

LIBS:=-lgcc -lplatform

all: $(APPNAME)

-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(SOURCES)))

$(APPNAME): $(OBJECTS) $(TOOLS)
	$(MAKE) $(MFLAGS) -Cplatform/$(PLATFORM)
	$(LD) $(OBJECTS) -o$(APPNAME) $(LDFLAGS) --start-group $(LIBS) --end-group
	$(OBJCOPY) -j .text -j .rodata -j .data -O binary $(APPNAME) $(APPNAME).rom
	truncate --size %4 $(APPNAME).rom
	$(NM) $(APPNAME) | tools/bin/makemap >>$(APPNAME).rom
	$(OBJCOPY) -i 2 -b 0 -I binary -O binary $(APPNAME).rom $(APPNAME).E.rom
	$(OBJCOPY) -i 2 -b 1 -I binary -O binary $(APPNAME).rom $(APPNAME).O.rom
	chmod -f -x $(APPNAME).E.rom $(APPNAME).O.rom $(APPNAME).rom || true

$(TOOLS):
	mkdir -p tools/bin
	$(MAKE) $(MFLAGS) -Ctools

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
	rm -rf $(DEPDIR) $(OBJDIR)
	$(MAKE) $(MFLAGS) -Cplatform/$(PLATFORM) clean
	$(MAKE) $(MFLAGS) -Ctools clean

