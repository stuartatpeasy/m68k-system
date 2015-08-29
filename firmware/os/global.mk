ARCH=m68k
PLATFORM=linux
FORMAT=elf

# for old toolchain
#
TRIPLET=$(ARCH)-$(FORMAT)

CC=$(ARCH)-$(FORMAT)-gcc
AS=$(ARCH)-$(FORMAT)-as
LD=$(ARCH)-$(FORMAT)-ld
CPP=$(ARCH)-$(FORMAT)-cpp
OBJCOPY=$(ARCH)-$(FORMAT)-objcopy
AR=$(ARCH)-$(FORMAT)-ar


# for gcc-5.2 toolchain
#
#TRIPLET=$(ARCH)-$(PLATFORM)-$(FORMAT)
#
#CC=$(TRIPLET)-gcc
#AS=$(TRIPLET)-as
#LD=$(TRIPLET)-ld
#CPP=$(TRIPLET)-cpp
#OBJCOPY=$(TRIPLET)-objcopy
#AR=$(TRIPLET)-ar

GCC_VERSION=$(shell $(CC) -v 2>&1 | grep "gcc version" | cut -d' ' -f3)


BASEDIR=/opt/m68k/lib/gcc/$(TRIPLET)/$(GCC_VERSION)

PROJECTDIR=/home/swallace/projects/m68k-system/firmware
SRCDIR=$(PROJECTDIR)/os
LIBCSWDIR=$(PROJECTDIR)/libc-sw

APPNAME=ayumos

CFLAGS=-I$(BASEDIR)/include -I$(SRCDIR) -I$(SRCDIR)/klibc -I. -c -Wall -O3 -m68000 -g \
        -DKMALLOC_HEAP -DTARGET_BIGENDIAN -DTARGET_MC68010 -ffreestanding -fomit-frame-pointer
AFLAGS=-m68000

