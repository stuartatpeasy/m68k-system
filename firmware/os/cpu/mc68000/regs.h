#ifndef CPU_MC68000_REGS_H
#define CPU_MC68000_REGS_H
/*
    Macros to make the GNU inline assembler syntax less horrific

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/

#define REG(r)      %r

#define a0      REG(a0)
#define a1      REG(a1)
#define a2      REG(a2)
#define a3      REG(a3)
#define a4      REG(a4)
#define a5      REG(a5)
#define a6      REG(a6)
#define a7      REG(a7)

#define fp      REG(fp)     /* = a6  */
#define sp      REG(sp)     /* = a7  */
#define usp     REG(usp)    /* = a7' */

#define d0      REG(d0)
#define d1      REG(d1)
#define d2      REG(d2)
#define d3      REG(d3)
#define d4      REG(d4)
#define d5      REG(d5)
#define d6      REG(d6)
#define d7      REG(d7)

#define sr      REG(sr)
#define ccr     REG(ccr)
#define vbr     REG(vbr)

#endif
