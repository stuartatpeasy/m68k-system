#ifndef CPU_MC68000_MC68000_H_INC
#define CPU_MC68000_MC68000_H_INC
/*
    Declarations relating to the Motorola 68000 CPU

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.
*/

#include <include/types.h>
#include <cpu/mc68000/exceptions.h>

#define CPU_MAX_IRQL        255

#include <cpu/cpu.h>

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

typedef struct regs regs_t;

/*
	MC68010 address/bus-error exception stack frame
*/
struct mc68010_address_exc_frame
{
	const u16 sr;
	const u32 pc;
	const u16 vector_offset;
	const u16 special_status_word;
	const u32 fault_addr;
	const u16 unused_reserved_1;
	const u16 data_output_buffer;
	const u16 unused_reserved_2;
	const u16 data_input_buffer;
	const u16 unused_reserved_3;
	const u16 instr_output_buffer;
	const u16 version_number;
	const u16 internal_information[15];
} __attribute__((packed));


/*
	MC68010 group 1/2 exception stack frame
*/
struct mc68010_exc_frame
{
    const u32 dummy;
	const u16 sr;
	const u32 pc;
	const u16 vector_offset;
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


extern void irq_handler(void);      /* defined in irq.S */


/*
	Default handler functions for all exceptions. See M68000 PRM page B-2 for information about
	this table and descriptions of each exception.
*/
void mc68000_exc_bus_error(int dummy, const struct mc68010_address_exc_frame f);
void mc68000_exc_address_error(int dummy, const struct mc68010_address_exc_frame f);

void mc68000_exc_generic(const struct mc68010_exc_frame f);


void mc68000_trap_0_handler(u16 irql, void *data, const regs_t regs);
void mc68000_trap_1_handler(u16 irql, void *data, const regs_t regs);
void mc68000_trap_2_handler(u16 irql, void *data, const regs_t regs);
void mc68000_trap_3_handler(u16 irql, void *data, const regs_t regs);
void mc68000_trap_4_handler(u16 irql, void *data, const regs_t regs);
void mc68000_trap_5_handler(u16 irql, void *data, const regs_t regs);
void mc68000_trap_6_handler(u16 irql, void *data, const regs_t regs);
void mc68000_trap_7_handler(u16 irql, void *data, const regs_t regs);
void mc68000_trap_8_handler(u16 irql, void *data, const regs_t regs);
void mc68000_trap_9_handler(u16 irql, void *data, const regs_t regs);
void mc68000_trap_10_handler(u16 irql, void *data, const regs_t regs);
void mc68000_trap_11_handler(u16 irql, void *data, const regs_t regs);
void mc68000_trap_12_handler(u16 irql, void *data, const regs_t regs);
void mc68000_trap_13_handler(u16 irql, void *data, const regs_t regs);
void mc68000_trap_14_handler(u16 irql, void *data, const regs_t regs);
void mc68000_trap_15_handler(u16 irql, void *data, const regs_t regs);


const char * const mc68000_dump_status_register(ku16 sr);
void mc68000_dump_regs(struct regs *regs);
void mc68010_dump_exc_frame(const struct mc68010_exc_frame * const f);
void mc68010_dump_address_exc_frame(const struct mc68010_address_exc_frame * const f);

#endif
