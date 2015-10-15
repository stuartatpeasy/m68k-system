/*
    Scheduler

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, July 2015.
*/

#include <kernel/sched.h>
#include <platform/platform.h>
#include <klibc/stdio.h>			/* TODO remove */

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
	plat_install_timer_irq_handler(irq_schedule, NULL);

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
            *(--process_stack_top) = (u32) 0xdeadbeef;  /* proc must exit with syscall, not rts */
            *(--process_stack_top) = (u32) arg;

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


void irq_schedule(ku32 irql, void *data, regs_t regs)
{
    /*
        This is the interrupt handler for the system timer interrupt, which triggers a context
        switch.  This function performs the context switch.
    */

    plat_stop_quantum();
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
    plat_start_quantum();

    /* Restore the incoming task's state from g_current_task. */

    memcpy(&regs, &g_current_proc->regs, sizeof(regs_t));

    cpu_enable_interrupts();   /* Not sure whether disable/enable IRQs is necessary */
}

