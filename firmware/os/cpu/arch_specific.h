#ifndef CPU_ARCH_SPECIFIC_H_INC
#define CPU_ARCH_SPECIFIC_H_INC
/*
    This header #includes CPU-architecture-specific declarations.

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.

    The CPU-architecture-specific file must declare/define the following things:

        CPU_MAX_IRQL            maximum IRQ number for the CPU (255 for the MC680x0 family)
*/

#ifndef IN_CPU_H
#error "This file should not be #include'd directly."
#endif

#if defined(TARGET_MC68000) || defined(TARGET_MC68010)
/*
    MC68000 / MC68010
*/

#define TARGET_BIGENDIAN

#include <cpu/mc68000/mc68000.h>
#else
/*
    Error: Undefined/unknown architecture
*/
#error No CPU-specific include file for the target architecture
#endif


#endif
