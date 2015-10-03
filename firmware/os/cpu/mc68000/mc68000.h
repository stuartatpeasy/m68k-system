#ifndef CPU_MC68000_MC68000_H_INC
#define CPU_MC68000_MC68000_H_INC
/*
    Declarations relating to the Motorola 68000 CPU

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.
*/

#include <include/types.h>

#define CPU_MAX_IRQL        255

#define VECTOR_TABLE_START      ((u32 *) 0x00000000)
#define VECTOR_TABLE_END        ((u32 *) 0x00000400)

/*
    struct regs - container for a CPU context
*/
struct regs
{
    reg32_t d[8];
    reg32_t a[8];
    reg16_t sr;     /* } These two elements align with the MC68000's exception stack frame. */
    reg32_t pc;     /* } See <cpu/mc68000/mc68000.h> for more information.                  */
} __attribute__((aligned(2),packed));


inline void cpu_nop(void)
{
    asm volatile("nop" :);
};


/*
    Interrupt enable/disable
*/

inline void cpu_enable_interrupts(void)
{
    /* Enable interrupts by setting the IRQ mask to 0 in the SR. */
    asm volatile
    (
        "andiw #0xf8ff, %%sr\n"
        :
        :
    );
};


inline void cpu_disable_interrupts(void)
{
    /* Disable interrupts by setting the IRQ mask to 7 in the SR. */
    asm volatile
    (
        "oriw #0x0700, %%sr\n"
        :
        :
    );
};


/*
    Byte-/word-swapping "intrinsics"

	FIXME - these don't work when used on memory operands, e.g. *p = bswap_16(*p).
	For some reason the compiler optimises away the writeback, or something.
*/

inline u16 bswap_16(u16 x)
{
    register u16 x_ = x;
    asm volatile
    (
        "rol.w #8, %w0\n"
        :
        : "d" (x_)
        : "d0", "cc"
    );

    return x_;
};


inline u32 bswap_32(u32 x)
{
    register u32 x_ = x;
    asm volatile
    (
        "rol.w #8, %0\n"
        "swap %0\n"
        "rol.w #8, %0"
        :
        : "d" (x_)
        : "cc"
    );

    return x_;
};


inline u32 wswap_32(u32 x)
{
    register u32 x_ = x;
    asm volatile
    (
        "swap %0\n"
        :
        : "d" (x_)
        : "cc"
    );

    return x_;
};

#endif
