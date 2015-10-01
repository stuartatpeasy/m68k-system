/*
    Scheduler

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, July 2015.
*/

#include "sched/sched.h"


proc_t g_ps[64];

#define NPROC_STRUCTS (sizeof(g_ps) / sizeof(g_ps[0]))
#define for_each_proc_struct(p) \
            for(p = g_ps; p < &(g_ps[NPROC_STRUCTS]); ++p)

proc_t *g_current_proc = NULL;
pid_t g_next_pid;
u32 g_ncontext_switches = 0;


void sched_init(void)
{
    proc_t *p;

    /* Install the scheduler IRQ handler */
    cpu_set_interrupt_handler(V_level_1_autovector, NULL, irq_schedule);

    for_each_proc_struct(p)
        p->state = ps_unborn;

    g_current_proc = &(g_ps[0]);
    g_next_pid = 1;
}


pid_t create_process(const s8* name, proc_main_t main_fn, u32 *arg, ku32 stack_len, ku16 flags)
{
    proc_t *p;
    pid_t retval = -1;      /* -1 indicates failure */

    cpu_disable_interrupts();

    for_each_proc_struct(p)
    {
        /* Find a vacant task_struct */
        if(p->state == ps_unborn)
        {
            /* TODO: work out how best to store name */
            /* FIXME - allocate this space in user RAM */
            u8 *process_stack = (u8 *) umalloc(stack_len);

            u32 *process_stack_top = (u32 *) (process_stack + stack_len);

printf("umalloc()ed process stack at %p\n", process_stack);
printf("stack top = %p\n", process_stack_top);
            p->id = g_next_pid++;
            p->parent = NULL;
            p->next = NULL;

            /* Register arrays might still contain values from a previous process; clear them */
            bzero(&(p->regs), sizeof(p->regs));

            /* Set up the process's initial stack */
            *(--process_stack_top) = (u32) arg;
            *(--process_stack_top) = (u32) 0xdeadbeef;  /* proc must exit with syscall, not rts */

            p->regs.a[7] = (u32) process_stack_top;
            p->regs.pc = (u32) main_fn;

            retval = p->id;
            p->state = ps_runnable; /* Mark process as runnable so scheduler will pick it up. */
            break;
        }
    }

    cpu_enable_interrupts();

    return retval;
}


void irq_schedule(u16 irql, void *data, regs_t regs)
{
    /*
        This is the interrupt handler for the system timer interrupt, which triggers a context
        switch.  This function performs the context switch.
    */

    // plat_stop_quantum();
    duart_stop_counter();
    cpu_disable_interrupts();   /* Not sure whether disable/enable IRQs is necessary */

    /* Store the outgoing process's state in *g_current_proc. */
    memcpy(&g_current_proc->regs, &regs, sizeof(regs_t));

	/*
        TODO: Decide which process to run next, and update g_current_proc accordingly
    */

    ++g_ncontext_switches;

    /*
        Start the next timeslice.  We need to do this before restoring the incoming tasks's state
        because the function call might interfere with register values.  This means that the next
        timeslice starts before the corresponding task is actually ready to run.
    */
//    plat_start_quantum();
    duart_start_counter();

    /* Restore the incoming task's state from g_current_task. */

    memcpy(&regs, &g_current_proc->regs, sizeof(regs_t));

    cpu_enable_interrupts();   /* Not sure whether disable/enable IRQs is necessary */
}


void sched_dump_proc(proc_t *ps)
{
    printf("D0=%08x  D1=%08x  D2=%08x  D3=%08x\n"
           "D4=%08x  D5=%08x  D6=%08x  D7=%08x\n"
           "A0=%08x  A1=%08x  A2=%08x  A3=%08x\n"
           "A4=%08x  A5=%08x  A6=%08x  SP=%08x\n\n"
           "PC=%08x  SR=%04x  [%s]\n",
           ps->regs.d[0], ps->regs.d[1], ps->regs.d[2], ps->regs.d[3],
           ps->regs.d[4], ps->regs.d[5], ps->regs.d[6], ps->regs.d[7],
           ps->regs.a[0], ps->regs.a[1], ps->regs.a[2], ps->regs.a[3],
           ps->regs.a[4], ps->regs.a[5], ps->regs.a[6], ps->regs.a[7],
           ps->regs.pc, ps->regs.sr, cpu_dump_status_register(ps->regs.sr));
}
