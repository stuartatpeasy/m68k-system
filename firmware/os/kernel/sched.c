/*
    Scheduler

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, July 2015.
*/

#include <kernel/sched.h>
#include <platform/platform.h>


//list_t g_run_queue;
//list_t g_wait_queue;

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
    p->next = p;
    p->prev = p;

    p->flags |= PROC_TYPE_KERNEL;
    strcpy(p->name, init_proc_name);

    g_current_proc = p;

    /* Install the scheduler IRQ handler */
	return plat_install_timer_irq_handler(cpu_context_switch);
}


/*
    sched() - select the next task to run, and update g_current_proc accordingly.
*/
void sched()
{
    /* Stop the current time-slice */
    plat_stop_quantum();

    ++g_current_proc->quanta;

    g_current_proc = g_current_proc->next;

    ++g_ncontext_switches;

    /*
        Start the next time-slice.  We need to do this before restoring the incoming task's state
        because the function call might interfere with register values.  This means that the next
        time-slice starts before the corresponding task is actually ready to run.
    */
    plat_start_quantum();
}
