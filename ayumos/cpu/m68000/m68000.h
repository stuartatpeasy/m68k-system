#ifndef CPU_M68000_M68000_H_INC
#define CPU_M68000_M68000_H_INC
/*
    Declarations relating to the Motorola 68000 CPU

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.
*/

#include <kernel/include/types.h>
#include <cpu/m68000/exceptions.h>
#include <cpu/m68000/primitives.h>

#define CPU_MAX_IRQL        255

#include <kernel/include/cpu.h>

#define VECTOR_TABLE_START      ((u32 *) 0x00000000)
#define VECTOR_TABLE_END        ((u32 *) 0x00000400)

/*
    struct regs - container for a CPU context

    NOTE: if this structure's size or layout changes, asm code will break!  Asm code elsewhere
    assumes that the length of this struct is 74.
*/
struct regs
{
    reg32_t usp;
    reg32_t d[8];
    reg32_t a[8];
    reg16_t sr;
    reg32_t pc;
} __attribute__((aligned(2),packed));   /* sizeof(struct regs) == 74 */

typedef struct regs regs_t;

/* Condition code register bits */
#define MC68K_CCR_X             BIT(4)          /* Extend flag                              */
#define MC68K_CCR_N             BIT(3)          /* Negative flag                            */
#define MC68K_CCR_Z             BIT(2)          /* Zero flag                                */
#define MC68K_CCR_V             BIT(1)          /* Overflow flag                            */
#define MC68K_CCR_C             BIT(0)          /* Carry flag                               */

/* Status register bits */
#define MC68K_SR_TRACE          BIT(15)         /* Trace mode enabled                       */
#define MC68K_SR_SUPERVISOR     BIT(13)         /* Supervisor/user flag: 1=S, 0=U           */
#define MC68K_SR_STACK          BIT(12)         /* Master/interrupt stack flag: 1=M, 0=I    */

#define MC68K_SR_X              MC68K_CCR_X     /* Extend flag                              */
#define MC68K_SR_N              MC68K_CCR_N     /* Negative flag                            */
#define MC68K_SR_Z              MC68K_CCR_Z     /* Zero flag                                */
#define MC68K_SR_V              MC68K_CCR_V     /* Overflow flag                            */
#define MC68K_SR_C              MC68K_CCR_C     /* Carry flag                               */

#define MC68K_SR_TRACE_SHIFT    (14)            /* } Mask and shift for current             */
#define MC68K_SR_TRACE_MASK     (0x3)           /* } trace level                            */

#define MC68K_SR_IPL_SHIFT      (8)             /* } Mask and shift for current             */
#define MC68K_SR_IPL_MASK       (0x7)           /* } interrupt priority level               */

/* Special status word (part of address/bus error stack frame) bits */
#define MC68K_SSW_RR            BIT(15)         /* Re-run flag: 0=CPU re-run; 1=software    */
#define MC68K_SSW_IF            BIT(13)         /* Instruction fetch flag                   */
#define MC68K_SSW_DF            BIT(12)         /* Data fetch flag                          */
#define MC68K_SSW_RM            BIT(11)         /* Read-modify-write cycle flag             */
#define MC68K_SSW_HB            BIT(10)         /* High-byte xfer from data-out -> data-in  */
#define MC68K_SSW_BY            BIT(9)          /* Byte-transfer flag                       */
#define MC68K_SSW_RW            BIT(8)          /* Read/write flag: 0=write; 1=read         */

#define MC68K_SSW_FC_MASK       (0x7)           /* } Mask and shift for function            */
#define MC68K_SSW_FC_SHIFT      (0)             /* } code used during faulted access        */

/* Extract IPL from SR value */
#define MC68K_SR_IPL(sr) \
    (((sr) >> MC68K_SR_IPL_SHIFT) & MC68K_SR_IPL_MASK)

/* Extrace trace level from SR value */
#define MC68K_SR_TRACE_LEVEL(sr) \
    (((sr) >> MC68K_SR_TRACE_SHIFT) & MC68K_SR_TRACE_MASK)

/*
    MC68010 group 1/2 exception stack frame
*/
typedef struct mc68010_short_exc_frame
{
    u16 sr;
    u32 pc;
    u16 format_offset;
} mc68010_short_exc_frame_t;


/*
    MC68010 address/bus-error exception stack frame.  NOTE: this struct does not include the SR
    and PC - they are extracted separately.
*/
typedef struct mc68010_address_exc_frame
{
    u16 sr;
    u32 pc;
    u16 format_offset;
    u16 special_status_word;
    u32 fault_addr;
    u16 unused_reserved_1;
    u16 data_output_buffer;
    u16 unused_reserved_2;
    u16 data_input_buffer;
    u16 unused_reserved_3;
    u16 instr_output_buffer;
    u16 version_number;
    u16 internal_information[15];
} mc68010_address_exc_frame_t;


/* Definitions for the "format code" field in an MC68010+ exception stack frame */
#define MC68K_EXC_FMT_SHORT     (0x0000)
#define MC68K_EXC_FMT_LONG      (0x8000)

inline void cpu_nop(void)
{
    asm volatile("nop" :);
}


/*
    Interrupt enable/disable
*/

inline void cpu_enable_interrupts(void)
{
    /* Enable interrupts by setting the IRQ mask to 0 in the SR. */
    asm volatile
    (
        "cpu_enable_interrupts_%=:  andiw #0xf8ff, %%sr     \n"
        :
        :
    );
}


inline void cpu_disable_interrupts(void)
{
    /* Disable interrupts by setting the IRQ mask to 7 in the SR. */
    asm volatile
    (
        "cpu_disable_interrupts_%=: oriw #0x0700, %%sr      \n"
        :
        :
    );
}


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
        "bswap_16_%=:               rol.w #8, %w0           \n"
        : /* "=d" (x_) */
        : "d" (x_)
        : "cc"
    );

    return x_;
}


inline u32 bswap_32(u32 x)
{
    register u32 x_ = x;
    asm volatile
    (
        "bswap_32_%=:               rol.w #8, %0            \n"
        "                           swap %0                 \n"
        "                           rol.w #8, %0            \n"
        :
        : "d" (x_)
        : "cc"
    );

    return x_;
}


inline u32 wswap_32(u32 x)
{
    register u32 x_ = x;
    asm volatile
    (
        "wswap_32_%=:               swap %0                 \n"
        :
        : "d" (x_)
        : "cc"
    );

    return x_;
}


/*
    cpu_tas() - atomically test and set a byte-sized memory location to 1, returning the previous
    contents of the location.
*/
inline u8 cpu_tas(u8 *addr)
{
    register u32 ret = 0;

    asm volatile
    (
        "cpu_tas_%=:                moveq    #0, %0         \n"
        "                           tas      %1             \n"
        "                           beq      cpu_tas_end_%= \n"
        "                           moveq    #1, %0         \n"
        "cpu_tas_end_%=:                                    \n"
        : "=&r" (ret)
        : "m" (*addr)
        : "cc"
    );

    return ret;
}


/* These are implemented in mc68000/irq.S */
extern void irq_router_full(void);
extern void irq_router_fast(void);
extern void irq_router_swi(void);

/* This is implemented in mc68000/syscall.S */
extern void syscall_dispatcher(void);

#endif
