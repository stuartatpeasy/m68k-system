# Makefile for ayumos
#
# Stuart Wallace <stuartw@atom.net>, 2011-2017.
#
#
include global.mk
include cpu/$(ARCH)/defs.mk

DFU=/usr/local/bin/dfu

DRIVER_SOURCES := ds17485.c encx24j600.c mc68681.c ps2controller.c

KERNEL_SOURCES := \
	device/ata.c device/auto.c device/block.c device/device.c device/memconsole.c device/nvram.c   \
	device/partition.c fs/file.c fs/mount.c fs/node.c fs/path.c fs/vfs.c fs/ext2/ext2.c            \
    fs/fat/fat.c fs/romfs.c boot.c console.c cpu.c elf.c entry.c error.c housekeeper.c keyboard.c  \
    ksym.c preempt.c process.c sched.c semaphore.c syscall.c tick.c user.c net/address.c net/arp.c \
    net/dhcp.c net/ethernet.c net/icmp.c net/interface.c net/ipv4.c net/net.c net/packet.c         \
    net/protocol.c net/raw.c net/route.c net/socket.c net/tcp.c net/tftp.c net/udp.c               \
    memory/buddy.c memory/extents.c memory/heap.c memory/kmalloc.c memory/slab.c util/bvec.c       \
    util/buffer.c util/checksum.c util/dump.c util/hash.c util/ktime.c util/numeric.c util/string.c

KERNEL_CXXSOURCES :=

KLIBC_SOURCES := \
	assert.c ctype.c errno.c printf.c stdio.c stdlib.c string.c strings.c

MONITOR_SOURCES := \
	commands.c disasm.c env.c history.c monitor.o readline.c srec.c

ASMSOURCES := $(addprefix cpu/$(ARCH)/,$(CPU_ASMSOURCES))

CSOURCES   := $(addprefix cpu/$(ARCH)/,$(CPU_CSOURCES)) \
  			  $(addprefix driver/,$(DRIVER_SOURCES))    \
              $(addprefix kernel/,$(KERNEL_SOURCES))    \
			  $(addprefix klibc/,$(KLIBC_SOURCES))      \
              $(addprefix monitor/,$(MONITOR_SOURCES))

CXXSOURCES := $(addprefix kernel/,$(KERNEL_CXXSOURCES))

SOURCES := $(CSOURCES) $(CXXSOURCES) $(ASMSOURCES)

LDSCRIPT := platform/$(PLATFORM)/kernel.ld

OBJECTS := $(addprefix $(OBJDIR)/,$(patsubst %.c,%.o,$(CSOURCES))    \
                                  $(patsubst %.cc,%.o,$(CXXSOURCES)) \
                                  $(patsubst %.S,%.o,$(ASMSOURCES)))

TOOLS := tools/bin/makemap

LIBS := -lgcc -lplatform

ROM := $(APPNAME).rom
ROM_EVEN := $(APPNAME).E.rom
ROM_ODD := $(APPNAME).O.rom

$(OBJDIR)/%.o : %.c $(DEPDIR)/%.d
	mkdir -p $(dir $@)
	echo "(TARGET_CC) $<"
	$(TARGET_CC) $(DEPFLAGS) $(TARGET_CFLAGS) -c -o$@ $<

$(OBJDIR)/%.o : %.cc $(DEPDIR)/%.d
	mkdir -p $(dir $@)
	echo "(TARGET_CXX) $<"
	$(TARGET_CXX) $(DEPFLAGS) $(TARGET_CXXFLAGS) -c -o$@ $<

$(OBJDIR)/%.o : %.S $(DEPDIR)/%.d
	mkdir -p $(dir $@)
	echo "(TARGET_AS) $<"
	$(TARGET_CC) $(DEPFLAGS) $(TARGET_CFLAGS) -c -o$@ $<

$(DEPDIR)/%.d:
	mkdir -p $(dir $@)

.PRECIOUS: $(DEPDIR)/%.d
.PRECIOUS: $(OBJDIR)/%.o

.PHONY: all
.PHONY: clean
.PHONY: dfu

all: $(APPNAME)

-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(SOURCES)))

$(APPNAME): $(OBJECTS) $(TOOLS)
	echo "=== building platform '$(PLATFORM)' ==="
	$(MAKE) $(MFLAGS) -Cplatform/$(PLATFORM)
	echo "=== creating images for '$(APPNAME)' ==="
	echo "(TARGET_LD) $(APPNAME)"
	$(TARGET_LD) $(OBJECTS) -o$(APPNAME) $(TARGET_LDFLAGS) --start-group $(LIBS) --end-group
	$(TARGET_OBJCOPY) -j .text -j .rodata -j .data -O binary $(APPNAME) $(ROM)
	truncate --size %4 $(ROM)
	echo "(TARGET_NM) $(ROM)"
	$(TARGET_NM) -g $(APPNAME) | tools/bin/makemap >> $(ROM)
	echo "(TARGET_OBJCOPY) $(ROM_EVEN)"
	$(TARGET_OBJCOPY) -i 2 -b 0 -I binary -O binary $(ROM) $(ROM_EVEN)
	echo "(TARGET_OBJCOPY) $(ROM_ODD)"
	$(TARGET_OBJCOPY) -i 2 -b 1 -I binary -O binary $(ROM) $(ROM_ODD)
	chmod -f -x $(ROM_EVEN) $(ROM_ODD) $(ROM) || true

$(TOOLS):
	echo "=== building tools ==="
	$(MAKE) $(MFLAGS) -Ctools

dfu:
	$(DFU) --program --baud 115200 --infile $(ROM)

clean:
	rm -f $(APPNAME) $(OBJECTS) $(ROM) $(ROM_EVEN) $(ROM_ODD)
	rm -rf $(DEPDIR) $(OBJDIR)
	$(MAKE) $(MFLAGS) -Cplatform/$(PLATFORM) clean
	$(MAKE) $(MFLAGS) -Ctools clean

