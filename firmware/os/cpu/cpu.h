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

/*
    "Intrinsic" functions
    ---------------------
*/
void cpu_halt(void);                            /* Halt processing, pending an interrupt        */
void cpu_reset(void) __attribute__((noreturn)); /* Reset the CPU                                */
void cpu_swi();                                 /* Raise a software IRQ (=TRAP #x on the 680x0) */

/*
    Atomically test and set the value at addr.  This is useful when implementing semaphores.  A
    return value of zero indicates that the value at addr was zero before being set (i.e. its
    value was changed by this operation); a nonzero retval indicates that the value at addr was
    already nonzero (i.e. its value was not changed).
*/
u8 cpu_tas(u8 *addr);

inline u16 bswap_16(u16 x);
inline u32 bswap_32(u32 x);
inline u32 wswap_32(u32 x);

/*
    Process-related declarations
    ----------------------------
*/

/* Initialise proc_t::regs prior to starting a new process */
s32 cpu_proc_init(regs_t *r, void *entry_point, void *ustack_top, void *kstack_top, ku32 flags);

/* This declaration is for an IRQ handler - it must not be called directly */
void cpu_context_switch();      /* Save context, call sched(), restore context      */

/*
    Called using C semantics with no args.  Save CPU state and use the caller's return address as
    the stored program counter value.  Call sched() to switch to a new process.  Enables the kernel
    to e.g. put a process to sleep and schedule a new process.
*/
void cpu_sleep_process();

/*
    Interrupt-related declarations
    ------------------------------
*/

#define IRQ_HANDLER_DEFAULT     (0x00000001)

/* IRQ handler function typedef */
typedef void (*irq_handler)(ku32 irql, void *data);

/* Initialise the IRQ handler table */
void cpu_irq_init_table();

/*
    Architecture-specific function: initialise CPU IRQ vector table, and set up any known handlers
    (e.g. the syscall handler).
*/
void cpu_irq_init_arch_specific(void);

/* Architecture-specific default IRQ handler function */
void cpu_default_irq_handler(ku32 irql, void *data);

/* Handler for all IRQs - calls installed IRQ-specific handlers in turn */
void cpu_irq_handler(ku32 irql);

/* Add a handler for the specified IRQ */
s32 cpu_irq_add_handler(ku32 irql, void *data, irq_handler handler);

/*
    An entry in the IRQ indirection table.  Consists of a ptr to a handler fn, and an arbitrary
    data item.
*/
typedef struct irq_handler_table_entry irq_handler_table_entry_t;

struct irq_handler_table_entry
{
    irq_handler                 handler;
    void *                      data;
    u32                         flags;
    irq_handler_table_entry_t * next;
};


/* The IRQ indirection table */
irq_handler_table_entry_t g_interrupt_handlers[CPU_MAX_IRQL];

s32 cpu_set_interrupt_handler(ku32 irql, void *data, irq_handler handler);

#undef IN_CPU_H

#endif
