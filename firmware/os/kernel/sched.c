/*
    Scheduler

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, July 2015.
*/

#include <kernel/sched.h>
#include <platform/platform.h>


u32 g_ncontext_switches = 0;
extern pid_t g_next_pid;

/*
    sched_init() - initialise the process scheduler, and convert the current thread of execution
    into a kernel process.
*/
s32 sched_init(const char * const init_proc_name)
{
    proc_t *p;

    p = CHECKED_KCALLOC(1, sizeof(proc_t));

    p->id = g_next_pid++;
    p->parent = NULL;
    p->state = ps_runnable;
    p->uid = ROOT_UID;
    p->gid = ROOT_GID;

    p->flags |= PROC_TYPE_KERNEL;
    strcpy(p->name, init_proc_name);

    list_insert(&p->queue, &g_run_queue);
    g_current_proc = p;

    /* Install the scheduler IRQ handler */
	return plat_install_timer_irq_handler(cpu_context_switch);
}


/*
    sched() - select the next task to run, and update g_current_proc accordingly.
*/
void sched()
{
    proc_t *g_prev_proc = g_current_proc;

    /* Stop the current time-slice */
    plat_stop_quantum();

    ++g_current_proc->quanta;

    if(list_is_last(&g_current_proc->queue, &g_run_queue))
        g_current_proc = list_first_entry(&g_run_queue, proc_t, queue);
    else
        g_current_proc = list_next_entry(g_current_proc, queue);

    /* If the process went to sleep, move it to the "sleeping" queue */
    if(g_prev_proc->state == ps_sleeping)
        list_move_insert(&g_prev_proc->queue, &g_sleep_queue);

   ++g_ncontext_switches;

    /*
        Start the next time-slice.  We need to do this before restoring the incoming task's state
        because the call to plat_start_quantum() will interfere with register values.  Consequently
        the next time-slice starts before the corresponding task is actually ready to run.
    */
    plat_start_quantum();
}
