ARCH=m68k

# for older toolchain
#FORMAT=elf

# for gcc-5.2 toolchain
FORMAT=linux-elf


TARGET_FMT=$(ARCH)-$(FORMAT)

CC=$(TARGET_FMT)-gcc
AS=$(TARGET_FMT)-as
LD=$(TARGET_FMT)-ld
CPP=$(TARGET_FMT)-cpp
OBJCOPY=$(TARGET_FMT)-objcopy
AR=$(TARGET_FMT)-ar
NM=$(TARGET_FMT)-nm

TARGET_GCC_VERSION=$(shell $(CC) --version 2>&1 | head -n 1 | cut -d' ' -f3)

HOSTCC=gcc
HOSTLD=ld

BASEDIR=/opt/m68k/lib/gcc/$(TARGET_FMT)/$(TARGET_GCC_VERSION)

PROJECTDIR=/home/swallace/projects/m68k-system/firmware
SRCDIR=$(PROJECTDIR)/os

APPNAME=ayumos

CFLAGS=-I$(SRCDIR) -I$(SRCDIR)/klibc -c -Wall -Wextra -O2 -m68000 -g -include buildcfg.h \
	-ffreestanding -fomit-frame-pointer -fno-delete-null-pointer-checks

AFLAGS=-m68000

PLATFORM=lambda_rev0
CPU=mc68000

