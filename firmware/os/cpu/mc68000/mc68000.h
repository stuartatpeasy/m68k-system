#ifndef CPU_MC68000_MC68000_H_INC
#define CPU_MC68000_MC68000_H_INC
/*
    Declarations relating to the Motorola 68000 CPU

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.
*/

#include <include/types.h>
#include <kernel/ksym.h>
#include <cpu/mc68000/exceptions.h>

#define CPU_MAX_IRQL        255

#include <cpu/cpu.h>

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

/* Extract IPL from SR value */
#define MC68K_SR_IPL(sr) \
    (((sr) >> MC68K_SR_IPL_SHIFT) & MC68K_SR_IPL_MASK)

/* Extrace trace level from SR value */
#define MC68K_SR_TRACE_LEVEL(sr) \
    (((sr) >> MC68K_SR_TRACE_SHIFT) & MC68K_SR_TRACE_MASK)


/*
	MC68010 address/bus-error exception stack frame.  NOTE: this struct does not include the SR
	and PC - they are extracted separately.
*/
typedef struct mc68010_address_exc_frame
{
	u16 vector_offset;
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


/*
	MC68010 group 1/2 exception stack frame
*/
typedef struct mc68010_short_exc_frame
{
	u16 sr;
	u32 pc;
	u16 format_offset;
} mc68010_short_exc_frame_t;

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
        "andiw #0xf8ff, %%sr\n"
        :
        :
    );
}


inline void cpu_disable_interrupts(void)
{
    /* Disable interrupts by setting the IRQ mask to 7 in the SR. */
    asm volatile
    (
        "oriw #0x0700, %%sr\n"
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
        "rol.w #8, %w0\n"
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
        "rol.w #8, %0\n"
        "swap %0\n"
        "rol.w #8, %0"
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
        "swap %0\n"
        :
        : "d" (x_)
        : "cc"
    );

    return x_;
}


/* These are implemented in mc68000/irq.S */
extern void irq_router_full(void);
extern void irq_router_fast(void);
extern void irq_router_swi(void);

/* This is implemented in mc68000/syscall.S */
extern void syscall_dispatcher(void);


const char * mc68000_dump_status_register(ku16 sr);
void mc68000_dump_regs(const regs_t *regs);
void mc68010_dump_address_exc_frame(mc68010_address_exc_frame_t *aef);

#endif
