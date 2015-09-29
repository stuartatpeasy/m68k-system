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

typedef void(*interrupt_handler)(u16 irql, const struct regs * regs);

interrupt_handler g_irq_handlers[CPU_MAX_IRQL];

void cpu_set_interrupt_handler(u16 irql, interrupt_handler handler);

#endif
