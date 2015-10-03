#ifndef CPU_CPU_H_INC
#define CPU_CPU_H_INC
/*
    Non-CPU-specific, but CPU-related, declarations

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.
*/

#define IN_CPU_H

#include <include/types.h>

/* Parse target-specific header */
#include <cpu/arch_specific.h>

typedef struct regs regs_t;     /* struct regs must hold a complete CPU context */

void cpu_halt(void);                            /* Halt processing, pending an interrupt        */
void cpu_reset(void) __attribute__((noreturn)); /* Reset the CPU                                */
void cpu_swi();                                 /* Raise a software IRQ (=TRAP #x on the 680x0) */
u32 cpu_tas(void *addr);                        /* Atomically test and set the value at addr    */

/*
    "Intrinsic" functions
*/
inline u16 bswap_16(u16 x);
inline u32 bswap_32(u32 x);
inline u32 wswap_32(u32 x);

/*
    Interrupt-related declarations
*/

/* Set a handler for a particular IRQ level */
typedef void(*interrupt_handler)(u16 irql, void *data, const regs_t regs);

/*
    An entry in the IRQ indirection table.  Consists of a ptr to a handler fn, and an arbitrary
    data item.
*/
typedef struct
{
    interrupt_handler   handler;
    void *              data;
} interrupt_handler_table_entry_t;


/* The IRQ indirection table */
interrupt_handler_table_entry_t g_interrupt_handlers[CPU_MAX_IRQL];

void cpu_set_interrupt_handler(u16 irql, void *data, interrupt_handler handler);

#undef IN_CPU_H

#endif
