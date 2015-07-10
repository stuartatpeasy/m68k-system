ARCH=m68k
FORMAT=coff

CC=$(ARCH)-$(FORMAT)-gcc
AS=$(ARCH)-$(FORMAT)-as
LD=$(ARCH)-$(FORMAT)-ld
CPP=$(ARCH)-$(FORMAT)-cpp
OBJCOPY=$(ARCH)-$(FORMAT)-objcopy
AR=$(ARCH)-$(FORMAT)-ar

BASEDIR=/opt/m68k/lib/gcc/$(ARCH)-$(FORMAT)/3.4.2

PROJECTDIR=/home/swallace/projects/m68k-system/firmware
SRCDIR=$(PROJECTDIR)/os
LIBCSWDIR=$(PROJECTDIR)/libc-sw

APPNAME=firmware

CFLAGS=-I$(BASEDIR)/include -I$(SRCDIR) -I$(SRCDIR)/klibc -I. -c -Wall -Os -m68000 -g \
        -DKMALLOC_HEAP -DTARGET_BIGENDIAN -DTARGET_MC68010 -ffreestanding -fomit-frame-pointer
AFLAGS=-m68000

