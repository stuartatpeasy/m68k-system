# CPU-specific makefile definitions
#
# Part of ayumos
#
# Stuart Wallace <stuartw@atom.net>, December 2016.
#
#
# This file defines the source files for this architecture, and the main makefile does the actual
# building.
#

CPU_CSOURCES = m68000.c

CPU_ASMSOURCES = syscall.S irq.S process.S

