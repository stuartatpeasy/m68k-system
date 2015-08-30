ARCH=m68k

# for older toolchain
FORMAT=elf

# for gcc-5.2 toolchain
#FORMAT=linux-elf


TARGET_FMT=$(ARCH)-$(FORMAT)

CC=$(TARGET_FMT)-gcc
AS=$(TARGET_FMT)-as
LD=$(TARGET_FMT)-ld
CPP=$(TARGET_FMT)-cpp
OBJCOPY=$(TARGET_FMT)-objcopy
AR=$(TARGET_FMT)-ar

GCC_VERSION=$(shell $(CC) --version 2>&1 | head -n 1 | cut -d' ' -f3)


BASEDIR=/opt/m68k/lib/gcc/$(TARGET_FMT)/$(GCC_VERSION)

PROJECTDIR=/home/swallace/projects/m68k-system/firmware
SRCDIR=$(PROJECTDIR)/os
LIBCSWDIR=$(PROJECTDIR)/libc-sw

APPNAME=ayumos

CFLAGS=-I$(BASEDIR)/include -I$(SRCDIR) -I$(SRCDIR)/klibc -I. -c -Wall -O3 -m68000 -g \
        -DKMALLOC_HEAP -DTARGET_BIGENDIAN -DTARGET_MC68010 -ffreestanding -fomit-frame-pointer
AFLAGS=-m68000

