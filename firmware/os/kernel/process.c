/*
    Functions relating to processes

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/

#include <include/list.h>
#include <kernel/process.h>
#include <kernel/sched.h>


list_t g_run_queue = LIST_INIT(g_run_queue);
list_t g_sleep_queue = LIST_INIT(g_sleep_queue);
list_t g_exited_queue = LIST_INIT(g_exited_queue);

proc_t *g_current_proc = NULL;
pid_t g_next_pid = 0;


/*
    proc_create() - create a new process and add it to the run queue.
*/
s32 proc_create(const uid_t uid, const gid_t gid, const s8* name, exe_img_t *img,
                proc_entry_fn_t entry, u32 *arg, ku32 stack_len, ku16 flags,
                const proc_t * const parent, pid_t *newpid)
{
    proc_t *p;
    u32 *ustack_top, *kstack_top;
    s32 ret;

    p = CHECKED_KCALLOC(1, sizeof(proc_t));

    /* Create process user stack */
    if(stack_len)
    {
        p->ustack = umalloc(stack_len);
        if(!p->ustack)
        {
            kfree(p);
            return ENOMEM;
        }
    }
    else
    {
        /* No user stack requested - fail unless we are creating a kernel process */
        if(!(flags & PROC_TYPE_KERNEL))
        {
            kfree(p);
            return EINVAL;
        }

        p->ustack = NULL;
    }

    /* Create process kernel stack */
    p->kstack = kmalloc(PROC_KSTACK_LEN);
    if(!p->kstack)
    {
        ufree(p->ustack);
        kfree(p);
        return ENOMEM;
    }

    ustack_top = (u32 *) ((u8 *) p->ustack + stack_len);
    kstack_top = (u32 *) ((u8 *) p->kstack + stack_len);

    p->id = g_next_pid++;
    p->exit_code = S32_MIN;
    p->parent = parent;
    p->uid = uid;
    p->gid = gid;
    p->img = img;
    p->arg = arg;

    list_init(&p->queue);

    strncpy(p->name, name, sizeof(p->name) - 1);
    p->name[sizeof(p->name) - 1] = '\0';

    if(entry == NULL)
        entry = img->entry_point;

    ret = cpu_proc_init(&p->regs, entry, ustack_top, kstack_top, flags);
    if(ret != SUCCESS)
    {
        kfree(p->kstack);
        ufree(p->ustack);
        kfree(p);
        return ret;
    }

    p->state = ps_runnable; /* Mark process as runnable so scheduler will pick it up. */

    if(newpid != NULL)
        *newpid = p->id;

    cpu_disable_interrupts();

    list_insert(&p->queue, &g_run_queue);

    cpu_enable_interrupts();

    return SUCCESS;
}


/*
    proc_get_pid() - return ID of currently-executing process
*/
pid_t proc_get_pid()
{
    return g_current_proc->id;
}


/*
    proc_current() - return ptr to currently-executing process
*/
proc_t *proc_current()
{
    return g_current_proc;
}


/*
    proc_do_exit() - terminate the current process and clean up.
*/
void proc_do_exit(s32 exit_code)
{
    proc_t * const g_exiting = g_current_proc;

    g_exiting->exit_code = exit_code;

    sched();

    /* Note: we're running in g_current_proc->next's quantum at this point... */

    /* Remove the exiting process from the run queue */
    list_delete(&g_exiting->queue);

    if(g_exiting->kstack != NULL)
        kfree(g_exiting->kstack);

    if(g_exiting->ustack != NULL)
        kfree(g_exiting->ustack);

    /* TODO: deallocate any other resources allocated by g_exiting (heaps, file handles, ...) */

    /* TODO: move this code into a generic exe_img_free() fn */
    if(g_exiting->img != NULL)
    {
        kfree(g_exiting->img->start);
        kfree(g_exiting->img);
    }

    /* TODO: move g_exiting onto "exited" list, to await exit-code collection? */

    kfree(g_exiting);
}


/*
    proc_sleep() - put the current process to sleep
*/
#include <klibc/stdio.h>    // FIXME remove
void proc_sleep()
{
    g_current_proc->state = ps_sleeping;    /* Causes process to be removed from run queue */

    /*
        Put the process to sleep.  This function will not return until the process wakes up.  In the
        meantime, another process will run.
    */
    cpu_switch_process();

    printf("Process %d woke up!\n", g_current_proc->id);        /* FIXME remove */
}


/*
    proc_wake_up() - ...
*/
void proc_wake_up()
{

}
