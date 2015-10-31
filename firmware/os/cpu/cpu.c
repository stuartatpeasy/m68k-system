/*
    Non-CPU-specific, but CPU-related, declarations

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.
*/

#include <cpu/cpu.h>
#include <include/defs.h>


interrupt_handler_table_entry_t g_interrupt_handlers[CPU_MAX_IRQL];


/*
    cpu_set_interrupt_handler() - register a handler function, and an arg, for an interrupt.
*/
s32 cpu_set_interrupt_handler(ku32 irql, void *data, interrupt_handler handler)
{
    if(irql > CPU_MAX_IRQL)
		return EINVAL;

    g_interrupt_handlers[irql].handler  = handler;
    g_interrupt_handlers[irql].data     = data;

	return SUCCESS;
}
