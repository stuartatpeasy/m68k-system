/*
    Scheduler

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, July 2015.
*/

#include <kernel/sched.h>
#include <platform/platform.h>
#include <klibc/stdio.h>			/* TODO remove */


//list_t g_run_queue;
//list_t g_wait_queue;

proc_t *g_current_proc = NULL;
pid_t g_next_pid = 0;
u32 g_ncontext_switches = 0;


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
    create_process() - create a new process and add it to the run queue.
*/
s32 create_process(const uid_t uid, const gid_t gid, const s8* name, proc_main_t main_fn, u32 *arg,
                   ku32 stack_len, ku16 flags, pid_t *newpid)
{
    proc_t *p;

    p = CHECKED_KCALLOC(1, sizeof(proc_t));

    u8 *process_stack = (u8 *) umalloc(stack_len);

    u32 *process_stack_top = (u32 *) (process_stack + stack_len);

    p->id = g_next_pid++;
    p->parent = NULL;
    p->uid = uid;
    p->gid = gid;

    strncpy(p->name, name, sizeof(p->name) - 1);
    p->name[sizeof(p->name) - 1] = '\0';

    /* Set up the process's initial stack */
    *(--process_stack_top) = (u32) 0xdeadbeef;  /* proc must exit with syscall, not rts */
    *(--process_stack_top) = (u32) arg;

    p->regs.a[7] = (u32) process_stack_top;
    p->regs.pc = (u32) main_fn;

    if(flags & PROC_TYPE_KERNEL)
        p->regs.sr |= 0x2000;       /* FIXME - arch-specific - force supervisor mode */

    p->state = ps_runnable; /* Mark process as runnable so scheduler will pick it up. */

    if(newpid != NULL)
        *newpid = p->id;

    cpu_disable_interrupts();

    p->prev = g_current_proc;
    p->next = g_current_proc->next;
    g_current_proc->next->prev = p;
    g_current_proc->next = p;

printf("created process; sp=%08x\n", p->regs.a[7]);
    cpu_enable_interrupts();

    return SUCCESS;
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


/*
    sched_yield() - called by a process that wishes to give up the rest of its quantum.
*/
void sched_yield()
{
    /*
        TODO:
            save process state

    */
}

