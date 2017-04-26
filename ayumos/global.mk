# Define the CPU architecture and the target platform
ARCH=m68000
PLATFORM=lambda

# TOOL_ROOT is the base directory in which the cross-compiler toolchain is installed
TOOL_ROOT=/opt/m68k
TOOL_BINDIR=$(TOOL_ROOT)/bin
TOOL_ARCH=m68k-elf
TOOL_PREFIX=$(TOOL_ARCH)-

# Set up paths to the cross-compiler binaries
TARGET_CC=$(TOOL_BINDIR)/$(TOOL_PREFIX)gcc
TARGET_CXX=$(TOOL_BINDIR)/$(TOOL_PREFIX)g++
TARGET_AS=$(TOOL_BINDIR)/$(TOOL_PREFIX)as
TARGET_LD=$(TOOL_BINDIR)/$(TOOL_PREFIX)ld
TARGET_CPP=$(TOOL_BINDIR)/$(TOOL_PREFIX)cpp
TARGET_OBJCOPY=$(TOOL_BINDIR)/$(TOOL_PREFIX)objcopy
TARGET_AR=$(TOOL_BINDIR)/$(TOOL_PREFIX)ar
TARGET_NM=$(TOOL_BINDIR)/$(TOOL_PREFIX)nm

TARGET_GCC_VERSION=$(shell $(TARGET_CC) --version 2>&1 | head -n 1 | cut -d' ' -f3)

# Paths to compiler and linker for the host architecture
CC=/usr/bin/gcc
LD=/usr/bin/ld

# This is what we're building
APPNAME=ayumos

# Path to libgcc.a for the target architecture
LIBGCCDIR=$(TOOL_ROOT)/lib/gcc/$(TOOL_ARCH)/$(TARGET_GCC_VERSION)/$(ARCH)

# Target compile/link options
TARGET_CFLAGS=-I. -Iklibc -Wall -Wextra -O2 -$(ARCH) -g -include buildcfg.h -ffreestanding \
              -fomit-frame-pointer -fno-delete-null-pointer-checks

# [2017-04-26] Exceptions are disabled in C++ code until I work out how to deal with the generated
# .eh_frame section.  Ditto RTTI, until I can work out how to link the necessary vtable.
TARGET_CXXFLAGS=$(TARGET_CFLAGS) -fno-exceptions -fno-rtti

TARGET_LDFLAGS=-g -T $(LDSCRIPT) -nostdlib -static -L$(LIBGCCDIR) -L.

# Standard directories
DEPDIR = .deps
OBJDIR = .obj

# Dependency-generation flags used by the compiler
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

ifndef VERBOSE
.SILENT:
endif
