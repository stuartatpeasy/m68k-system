# Makefile for ayumos
#
# Stuart Wallace <stuartw@atom.net>, 2011-2016.
#
#
include global.mk
include cpu/$(ARCH)/defs.mk

DFU=/usr/local/bin/dfu

DRIVER_SOURCES := ds17485.c encx24j600.c mc68681.c ps2controller.c

KERNEL_SOURCES := \
	device/ata.c device/auto.c device/block.c device/device.c device/memconsole.c device/nvram.c \
	device/partition.c fs/file.c fs/mount.c fs/vfs.c fs/ext2/ext2.c fs/fat/fat.c fs/romfs.c \
	boot.c console.c cpu.c elf.c entry.c error.c housekeeper.c keyboard.c ksym.c preempt.c \
	process.c sched.c semaphore.c syscall.c tick.c user.c net/address.c net/arp.c net/dhcp.c \
	net/ethernet.c net/icmp.c net/interface.c net/ipv4.c net/net.c net/packet.c net/protocol.c \
	net/raw.c net/route.c net/socket.c net/tcp.c net/tftp.c net/udp.c memory/buddy.c \
	memory/extents.c memory/heap.c memory/kmalloc.c memory/slab.c util/bvec.c util/buffer.c \
	util/checksum.c util/dump.c util/hash.c util/ktime.c util/numeric.c util/string.c

KLIBC_SOURCES := \
	assert.c ctype.c errno.c printf.c stdio.c stdlib.c string.c strings.c

MONITOR_SOURCES := \
	commands.c disasm.c env.c history.c monitor.o readline.c srec.c

ASMSOURCES := $(addprefix cpu/$(ARCH)/,$(CPU_ASMSOURCES))

CSOURCES := $(addprefix cpu/$(ARCH)/,$(CPU_CSOURCES)) \
			$(addprefix driver/,$(DRIVER_SOURCES)) \
            $(addprefix kernel/,$(KERNEL_SOURCES)) \
			$(addprefix klibc/,$(KLIBC_SOURCES)) \
            $(addprefix monitor/,$(MONITOR_SOURCES))

SOURCES := $(CSOURCES) $(ASMSOURCES)

LDSCRIPT := platform/$(PLATFORM)/kernel.ld

DEPDIR := .deps
OBJDIR := .obj

$(shell mkdir -p $(DEPDIR) $(OBJDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

OBJECTS := $(addprefix $(OBJDIR)/,$(patsubst %.c,%.o,$(CSOURCES)) $(patsubst %.S,%.o,$(ASMSOURCES)))

$(OBJDIR)/%.o : %.c $(DEPDIR)/%.d
	@mkdir -p $(dir $@)
	$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -c -o$@ $<

$(OBJDIR)/%.o : %.S $(DEPDIR)/%.d
	@mkdir -p $(dir $@)
	$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -c -o$@ $<

$(DEPDIR)/%.d:
	@mkdir -p $(dir $@)

.PRECIOUS: $(DEPDIR)/%.d
.PRECIOUS: $(OBJDIR)/%.o

TOOLS := tools/bin/makemap

LDFLAGS := -g -T $(LDSCRIPT) -nostdlib -static -L$(LIBGCCDIR) -L.

LIBS := -lgcc -lplatform

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

dfu:
	$(DFU) --program --baud 115200 --infile $(APPNAME).rom

clean:
	rm -f $(APPNAME) $(OBJECTS) $(APPNAME).?.rom $(APPNAME).rom
	rm -rf $(DEPDIR) $(OBJDIR)
	$(MAKE) $(MFLAGS) -Cplatform/$(PLATFORM) clean
	$(MAKE) $(MFLAGS) -Ctools clean

