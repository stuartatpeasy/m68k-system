#ifndef CPU_CPU_H_INC
#define CPU_CPU_H_INC
/*
    Non-CPU-specific, but CPU-related, declarations

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.
*/

#include <include/types.h>

/* FIXME - should be defined for the specific CPU */
#define CPU_MAX_IRQL        255

typedef u32 reg32_t;
typedef u16 reg16_t;

struct regs;        /* Forward declaration - will be refined by CPU-specific code */

/* Set a handler for a particular IRQ level */
typedef void(*interrupt_handler)(u16 irql, void *data, const struct regs * regs);

typedef struct
{
    interrupt_handler   handler;
    void *              data;
} interrupt_handler_table_entry_t;


interrupt_handler_table_entry_t g_interrupt_handlers[CPU_MAX_IRQL];

void cpu_set_interrupt_handler(u16 irql, void *data, interrupt_handler handler);

#endif
