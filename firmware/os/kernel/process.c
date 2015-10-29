/*
    Functions relating to processes

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/

#include <kernel/process.h>
#include <kernel/sched.h>

proc_t *g_current_proc = NULL;
pid_t g_next_pid = 0;


/*
    proc_create() - create a new process and add it to the run queue.
*/
s32 proc_create(const uid_t uid, const gid_t gid, const s8* name, exe_img_t *img, u32 *arg,
                ku32 stack_len, ku16 flags, const proc_t * const parent, pid_t *newpid)
{
    proc_t *p;
    u32 *ustack_top, *kstack_top;
    s32 ret;

    p = CHECKED_KCALLOC(1, sizeof(proc_t));

    /* Create process user stack */
    p->ustack = umalloc(stack_len);
    if(!p->ustack)
    {
        kfree(p);
        return ENOMEM;
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

    strncpy(p->name, name, sizeof(p->name) - 1);
    p->name[sizeof(p->name) - 1] = '\0';

    ret = cpu_proc_init(&p->regs, img->entry_point, ustack_top, kstack_top, flags);
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

    p->prev = g_current_proc;
    p->next = g_current_proc->next;
    g_current_proc->next->prev = p;
    g_current_proc->next = p;

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
    g_exiting->next->prev = g_exiting->prev;
    g_exiting->prev->next = g_exiting->next;

    kfree(g_exiting->kstack);
    kfree(g_exiting->ustack);

    /* TODO: deallocate any other resources allocated by g_exiting (heaps, file handles, ...) */

    /* TODO: move this code into a generic exe_img_free() fn */
    kfree(g_exiting->img->start);
    kfree(g_exiting->img);

    /* TODO: move g_exiting onto "exited" list, to await exit-code collection? */

    kfree(g_exiting);
}
