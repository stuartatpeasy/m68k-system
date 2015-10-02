#ifndef CPU_MC68000_MC68000_H_INC
#define CPU_MC68000_MC68000_H_INC
/*
    Declarations relating to the Motorola 68000 CPU

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.
*/

#include <include/types.h>

#define VECTOR_TABLE_START      ((u32 *) 0x00000000)
#define VECTOR_TABLE_END        ((u32 *) 0x00000400)


struct regs
{
    reg32_t d[8];
    reg32_t a[8];
    reg16_t sr;     /* } These two elements align with the MC68000's exception stack frame. */
    reg32_t pc;     /* } See <cpu/mc68000/mc68000.h> for more information.                  */
} __attribute__((aligned(2),packed));

#endif
