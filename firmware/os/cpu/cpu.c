/*
    Non-CPU-specific, but CPU-related, declarations

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.
*/

#include <cpu/cpu.h>

interrupt_handler_table_entry_t g_interrupt_handlers[CPU_MAX_IRQL];


void cpu_set_interrupt_handler(u16 irql, void *data, interrupt_handler handler)
{
    if(irql < CPU_MAX_IRQL)
    {
        g_interrupt_handlers[irql].handler  = handler;
        g_interrupt_handlers[irql].data     = data;
    }
}
