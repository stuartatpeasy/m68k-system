# Define the CPU architecture and the target platform
ARCH=m68000
PLATFORM=lambda

# TOOLDIR is the base directory in which the cross-compiler toolchain is installed
TOOLDIR=/opt/m68k
TOOLPREFIX=m68k-elf

# Set up paths to the cross-compiler binaries
CC=$(TOOLDIR)/bin/$(TOOLPREFIX)-gcc
AS=$(TOOLDIR)/bin/$(TOOLPREFIX)-as
LD=$(TOOLDIR)/bin/$(TOOLPREFIX)-ld
CPP=$(TOOLDIR)/bin/$(TOOLPREFIX)-cpp
OBJCOPY=$(TOOLDIR)/bin/$(TOOLPREFIX)-objcopy
AR=$(TOOLDIR)/bin/$(TOOLPREFIX)-ar
NM=$(TOOLDIR)/bin/$(TOOLPREFIX)-nm

TARGET_GCC_VERSION=$(shell $(CC) --version 2>&1 | head -n 1 | cut -d' ' -f3)

# Paths to compiler and linker for the host architecture
HOSTCC=/usr/bin/gcc
HOSTLD=/usr/bin/ld

# This is what we're building
APPNAME=ayumos

# Path to libgcc.a for the target architecture
LIBGCCDIR=$(TOOLDIR)/lib/gcc/$(TOOLPREFIX)/$(TARGET_GCC_VERSION)/$(ARCH)

CFLAGS=-I. -Iklibc -Wall -Wextra -O2 -$(ARCH) -g -include buildcfg.h -ffreestanding \
       -fomit-frame-pointer -fno-delete-null-pointer-checks

AFLAGS=-$(ARCH)

ifndef VERBOSE
.SILENT:
endif
