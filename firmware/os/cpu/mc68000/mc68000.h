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
    assumes that the length of this struct is 70.
*/
struct regs
{
    reg32_t d[8];
    reg32_t a[8];
    reg16_t sr;     /* } These two elements align with the MC68000's exception stack frame. */
    reg32_t pc;     /* } See <cpu/mc68000/mc68000.h> for more information.                  */
} __attribute__((aligned(2),packed));   /* sizeof(struct regs) == 70 */

typedef struct regs regs_t;

/*
	MC68010 address/bus-error exception stack frame.  NOTE: this struct does not include the SR
	and PC - they are extracted separately.
*/
struct mc68010_address_exc_frame
{
	ku16 vector_offset;
	ku16 special_status_word;
	ku32 fault_addr;
	ku16 unused_reserved_1;
	ku16 data_output_buffer;
	ku16 unused_reserved_2;
	ku16 data_input_buffer;
	ku16 unused_reserved_3;
	ku16 instr_output_buffer;
	ku16 version_number;
	ku16 internal_information[15];
} __attribute__((packed));


/*
	MC68010 group 1/2 exception stack frame
*/
struct mc68010_exc_frame
{
    ku32 dummy;
	ku16 sr;
	ku32 pc;
	ku16 vector_offset;
} __attribute__((packed));


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


/* These are implemented in mc68000/irq.S */
extern void irq_router_full(void);
extern void irq_router_fast(void);
extern void irq_router_swi(void);


/*
	Default handler functions for all exceptions. See M68000 PRM page B-2 for information about
	this table and descriptions of each exception.
*/
void mc68000_exc_bus_error(ku32 irql, void *data, const regs_t regs);
void mc68000_exc_address_error(ku32 irql, void *data, const regs_t regs);

void mc68000_exc_generic(ku32 irql, void *data, const regs_t regs);


void mc68000_trap_0_handler(ku32 irql, void *data, const regs_t regs);
void mc68000_trap_1_handler(ku32 irql, void *data, const regs_t regs);
void mc68000_trap_2_handler(ku32 irql, void *data, const regs_t regs);
void mc68000_trap_3_handler(ku32 irql, void *data, const regs_t regs);
void mc68000_trap_4_handler(ku32 irql, void *data, const regs_t regs);
void mc68000_trap_5_handler(ku32 irql, void *data, const regs_t regs);
void mc68000_trap_6_handler(ku32 irql, void *data, const regs_t regs);
void mc68000_trap_7_handler(ku32 irql, void *data, const regs_t regs);
void mc68000_trap_8_handler(ku32 irql, void *data, const regs_t regs);
void mc68000_trap_9_handler(ku32 irql, void *data, const regs_t regs);
void mc68000_trap_10_handler(ku32 irql, void *data, const regs_t regs);
void mc68000_trap_11_handler(ku32 irql, void *data, const regs_t regs);
void mc68000_trap_12_handler(ku32 irql, void *data, const regs_t regs);
void mc68000_trap_13_handler(ku32 irql, void *data, const regs_t regs);
void mc68000_trap_14_handler(ku32 irql, void *data, const regs_t regs);
void mc68000_trap_15_handler(ku32 irql, void *data, const regs_t regs);


const char * const mc68000_dump_status_register(ku16 sr);
void mc68000_dump_regs(const regs_t *regs);
void mc68010_dump_exc_frame(ku32 irql, const regs_t * const regs);
void mc68010_dump_address_exc_frame(ku32 irql, const regs_t * const regs);

#endif
