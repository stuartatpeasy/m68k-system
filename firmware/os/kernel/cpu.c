/*
    Non-CPU-specific, but CPU-related, declarations

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.
*/

#include <kernel/cpu.h>
#include <kernel/include/defs.h>


irq_handler_table_entry_t g_irq_handlers[CPU_MAX_IRQL];


/*
    cpu_irq_init_table() - initialise the IRQ handler table
*/
void cpu_irq_init_table()
{
    irq_handler_table_entry_t *irqent,
                              *const irqent_last = &g_irq_handlers[CPU_MAX_IRQL];

    for(irqent = g_irq_handlers; irqent < irqent_last; ++irqent)
    {
        irqent->handler = cpu_default_irq_handler;
        irqent->data    = NULL;
        irqent->flags   = IRQ_HANDLER_DEFAULT;
        irqent->next    = NULL;
    }

    /*
        Perform architecture-specific init, e.g. init CPU IRQ vector table, set any specific
        handler functions, etc.
    */
    cpu_irq_init_arch_specific();
}


/*
    cpu_irq_add_handler() - register a handler function, and an arg, for an interrupt.
*/
s32 cpu_irq_add_handler(ku32 irql, void *data, irq_handler handler)
{
    irq_handler_table_entry_t *ent;

    if(irql > CPU_MAX_IRQL)
		return EINVAL;

    ent = &g_irq_handlers[irql];

    /*
        If the specified irql has a default handler (flags & IRQ_HANDLER_DEFAULT), replace it with
        the supplied handler.  Otherwise append the supplied handler to the chain of handlers.
    */
    if(!(ent->flags & IRQ_HANDLER_DEFAULT))
    {
        /* Append the specified IRQ handler to the chain of handlers */
        while(ent->next != NULL)
            ent = ent->next;

        ent->next = CHECKED_KMALLOC(sizeof(irq_handler_table_entry_t));
        ent = ent->next;
    }

    ent->handler = handler;
    ent->data = data;
    ent->flags = 0;
    ent->next = NULL;

	return SUCCESS;
}


/*
    cpu_irq_handler() - entry point for all interrupts.  Calls each associated interrupt handler in
    turn.
*/
void cpu_irq_handler(ku32 irql)
{
    irq_handler_table_entry_t *ent = &g_irq_handlers[irql];

    do
    {
        ent->handler(irql, ent->data);
        ent = ent->next;
    } while(ent != NULL);
}
