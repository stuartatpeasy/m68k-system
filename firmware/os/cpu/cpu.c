/*
    Non-CPU-specific, but CPU-related, declarations

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.
*/

#include <cpu/cpu.h>

interrupt_handler g_irq_handlers[CPU_MAX_IRQL];

void cpu_set_interrupt_handler(u16 irql, interrupt_handler handler)
{
    if(irql < CPU_MAX_IRQL)
        g_irq_handlers[irql] = handler;
}
