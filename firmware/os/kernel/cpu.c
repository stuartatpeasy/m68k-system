/*
    Non-CPU-specific, but CPU-related, declarations

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.
*/

#include <kernel/cpu.h>
#include <kernel/include/defs.h>


irq_handler_table_entry_t g_irq_handlers[CPU_MAX_IRQL];

s32 cpu_irq_set_default_handler(ku32 irql);

/*
    cpu_irq_init_table() - initialise the IRQ handler table
*/
void cpu_irq_init_table()
{
    u32 irql;

    for(irql = 0; irql <= CPU_MAX_IRQL; ++irql)
        cpu_irq_set_default_handler(irql);

    /*
        Perform architecture-specific init, e.g. init CPU IRQ vector table, set any specific
        handler functions, etc.
    */
    cpu_irq_init_arch_specific();
}


/*
    cpu_irq_set_default_handler() - install the default handler for the specified IRQ level
*/
s32 cpu_irq_set_default_handler(ku32 irql)
{
    irq_handler_table_entry_t *ent;

    if((irql == IRQL_NONE) || (irql > CPU_MAX_IRQL))
        return EINVAL;

    ent = &g_irq_handlers[irql];

    ent->handler    = cpu_default_irq_handler;
    ent->data       = NULL;
    ent->flags      = IRQ_HANDLER_DEFAULT;
    ent->next       = NULL;

    return SUCCESS;
}


/*
    cpu_irq_add_handler() - register a handler function, and an arg, for an interrupt.
*/
s32 cpu_irq_add_handler(ku32 irql, void *data, irq_handler handler)
{
    irq_handler_table_entry_t *ent;

    if((irql == IRQL_NONE) || (irql > CPU_MAX_IRQL))
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
    cpu_irq_remove_handler() - de-register (remove) an interrupt handler function.  Locate the
    handler function by looking for a matching irq_handler; if the data arg is not NULL, also match
    on the value of the data argument associated with the handler.
*/
s32 cpu_irq_remove_handler(ku32 irql, irq_handler handler, void *data)
{
    irq_handler_table_entry_t *ent, *ent_prev;

    if((irql == IRQL_NONE) || (irql > CPU_MAX_IRQL))
        return EINVAL;

    for(ent_prev = NULL, ent = &g_irq_handlers[irql]; ent; ent_prev = ent, ent = ent->next)
    {
        /* Look for matching handler function and (optionally) a match on the data arg */
        if((ent->handler == handler) && ((data == NULL) || (data == ent->data)))
        {
            if(ent_prev)
            {
                /* This is not the first handler in a chain */
                ent_prev->next = ent->next;
                kfree(ent);
                return SUCCESS;
            }
            else
            {
                /* This is the first handler in a chain */
                if(ent->next)
                {
                    /*
                        There are subsequent handlers: replace the handler with the next one in the
                        chain, then free the memory associated with the next handler.
                    */
                    irq_handler_table_entry_t *next = ent->next;

                    *ent = *next;

                    kfree(next);
                    return SUCCESS;
                }
                else
                {
                    /* There are no subsequent handlers: install the default handler */
                    return cpu_irq_set_default_handler(irql);
                }
            }
        }
    }

    return ENOENT;
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
