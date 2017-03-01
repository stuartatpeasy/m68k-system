# Define the CPU architecture and the target platform
ARCH=m68000
PLATFORM=lambda

# TOOLDIR is the base directory in which the cross-compiler toolchain is installed
TOOLDIR=/opt/m68k
TOOLPREFIX=m68k-elf

# Set up paths to the cross-compiler binaries
TARGET_CC=$(TOOLDIR)/bin/$(TOOLPREFIX)-gcc
TARGET_AS=$(TOOLDIR)/bin/$(TOOLPREFIX)-as
TARGET_LD=$(TOOLDIR)/bin/$(TOOLPREFIX)-ld
TARGET_CPP=$(TOOLDIR)/bin/$(TOOLPREFIX)-cpp
TARGET_OBJCOPY=$(TOOLDIR)/bin/$(TOOLPREFIX)-objcopy
TARGET_AR=$(TOOLDIR)/bin/$(TOOLPREFIX)-ar
TARGET_NM=$(TOOLDIR)/bin/$(TOOLPREFIX)-nm

TARGET_GCC_VERSION=$(shell $(TARGET_CC) --version 2>&1 | head -n 1 | cut -d' ' -f3)

# Paths to compiler and linker for the host architecture
CC=/usr/bin/gcc
LD=/usr/bin/ld

# This is what we're building
APPNAME=ayumos

# Path to libgcc.a for the target architecture
LIBGCCDIR=$(TOOLDIR)/lib/gcc/$(TOOLPREFIX)/$(TARGET_GCC_VERSION)/$(ARCH)

# Target compile/link options
TARGET_CFLAGS=-I. -Iklibc -Wall -Wextra -O2 -$(ARCH) -g -include buildcfg.h -ffreestanding \
              -fomit-frame-pointer -fno-delete-null-pointer-checks -floop-unroll-and-jam

TARGET_LDFLAGS=-g -T $(LDSCRIPT) -nostdlib -static -L$(LIBGCCDIR) -L.

# Standard directories
DEPDIR = .deps
OBJDIR = .obj

# Dependency-generation flags used by the compiler
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

ifndef VERBOSE
.SILENT:
endif
