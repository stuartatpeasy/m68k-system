/*
    Functions relating to processes

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/

#include <kernel/include/process.h>
#include <kernel/include/fs/path.h>
#include <kernel/include/limits.h>
#include <kernel/include/memory/kmalloc.h>
#include <kernel/include/preempt.h>
#include <kernel/include/sched.h>
#include <klibc/include/stdlib.h>
#include <klibc/include/string.h>


list_t g_run_queue = LIST_INIT(g_run_queue);
list_t g_sleep_queue = LIST_INIT(g_sleep_queue);
list_t g_exited_queue = LIST_INIT(g_exited_queue);

proc_t *g_current_proc = NULL;
pid_t g_next_pid = 0;
extern time_t g_current_timestamp;


/*
    proc_create() - create a new process and add it to the run queue.
*/
s32 proc_create(const uid_t uid, const gid_t gid, const s8* name, exe_img_t *img,
                proc_entry_fn_t entry, void *arg, ku32 stack_len, ku16 flags, ks8 *wd,
                const proc_t * const parent, pid_t *newpid)
{
    proc_t *p;
    u32 *ustack_top, *kstack_top;
    s32 ret;

    /* Validate supplied working directory, if any */
    if((wd != PROC_DEFAULT_WD) && !path_is_absolute(wd))
        return -EINVAL;

    p = CHECKED_KCALLOC(1, sizeof(proc_t));

    /* Create process user stack */
    if(stack_len)
    {
        p->ustack = umalloc(stack_len);
        if(!p->ustack)
        {
            kfree(p);
            return -ENOMEM;
        }
    }
    else
    {
        /* No user stack requested - fail unless we are creating a kernel process */
        if(!(flags & PROC_TYPE_KERNEL))
        {
            kfree(p);
            return -EINVAL;
        }

        p->ustack = NULL;
    }

    /* Create process kernel stack */
    p->kstack = kmalloc(PROC_KSTACK_LEN);
    if(!p->kstack)
    {
        ufree(p->ustack);
        kfree(p);
        return -ENOMEM;
    }

    /* Set up the initial working directory for the new process */
    if(wd == PROC_DEFAULT_WD)
    {
        /*
            By default, a process inherits its working directory from its parent.  If the process
            has no parent, the default working directory is the root directory.
        */
        p->cwd = strdup((parent == NULL) ? ROOT_DIR : proc_getcwd(parent));

    }
    else
    {
        s8 *wd_canon = strdup(wd);

        if(wd_canon)
        {
            path_canonicalise(wd_canon);
            p->cwd = strdup(wd_canon);
            kfree(wd_canon);
        }
    }

    /* If p->cwd is not set at this point, we ran out of memory doing a strdup() above */
    if(!p->cwd)
    {
        ufree(p->ustack);
        kfree(p->kstack);
        kfree(p);
        return -ENOMEM;
    }

    ustack_top = (u32 *) ((u8 *) p->ustack + stack_len);
    kstack_top = (u32 *) ((u8 *) p->kstack + PROC_KSTACK_LEN);

    p->exit_code = S32_MIN;
    p->default_perm = PROC_DEFAULT_FILE_PERM;
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

    ret = cpu_proc_init(&p->regs, entry, arg, ustack_top, kstack_top, flags);
    if(ret != SUCCESS)
    {
        kfree(p->cwd);
        kfree(p->kstack);
        ufree(p->ustack);
        kfree(p);
        return ret;
    }

    p->state = ps_runnable; /* Mark process as runnable so scheduler will pick it up. */

    preempt_disable();

    p->id = g_next_pid++;
    list_insert(&p->queue, &g_run_queue);

    preempt_enable();

    if(newpid != NULL)
        *newpid = p->id;

    return SUCCESS;
}


/*
    proc_get_pid() - return ID of currently-executing process
*/
pid_t proc_get_pid()
{
    return (g_current_proc != NULL) ? g_current_proc->id : 0;
}


/*
    proc_current() - return ptr to currently-executing process
*/
proc_t *proc_current()
{
    return g_current_proc;
}


/*
    proc_current_uid() - return the user ID of the current process.
*/
uid_t proc_current_uid()
{
    return g_current_proc->uid;
}


/*
    proc_current_gid() - return the group ID of the current process.
*/
gid_t proc_current_gid()
{
    return g_current_proc->gid;
}


/*
    proc_current_default_perm() - return a file_perm_t object representing the default permissions
    for files created by the current process.
*/
file_perm_t proc_current_default_perm()
{
    return g_current_proc->default_perm;
}


/*
    proc_getcwd() - get the current working directory of a process.  If <proc> is NULL, return the
    cwd of the currently-executing process.  Otherwise, return the cwd of the specified process.
*/
ks8 *proc_getcwd(const proc_t *proc)
{
    return (proc == NULL) ? g_current_proc->cwd : proc->cwd;
}


/*
    proc_setcwd() - set the current working directory of a process.  If <proc> is NULL, set the cwd
    for the currently-executing process.
*/
s32 proc_setcwd(proc_t *proc, ks8 *dir)
{
    s8 *new_cwd = strdup((dir == PROC_DEFAULT_WD) ? ROOT_DIR : dir);
    if(!new_cwd)
        return -ENOMEM;

    if(proc == NULL)
        proc = proc_current();

    kfree(proc->cwd);
    proc->cwd = new_cwd;

    return SUCCESS;
}


/*
    proc_destroy() - terminate the current process and clean up.
*/
void proc_destroy(s32 exit_code)
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
void proc_sleep()
{
    g_current_proc->state = ps_sleeping;    /* Causes process to be removed from run queue */

    /*
        Put the process to sleep.  This function will not return until the process wakes up.  In the
        meantime, another process will run.
    */
    cpu_switch_process();
}


/*
    proc_sleep_for() -"sleep" the current process for the specified number of seconds.  Note that
    the process does not actually move on to the sleep queue during this time - instead it
    repeatedly yields its quantum until the wakeup time arrives.  This could be improved.
*/
void proc_sleep_for(s32 secs)
{
    const time_t wakeup_time = g_current_timestamp + secs;

    while(g_current_timestamp < wakeup_time)
        cpu_switch_process();
}


/*
    proc_sleep_until() - "sleep" the current process until the specified timestamp is passed.  Note
    that the process does not actually move on to the sleep queue during this time - instead it
    repeatedly yields its quantum until the wakeup time arrives.  This could be improved.
*/
void proc_sleep_until(s32 when)
{
    while(g_current_timestamp < when)
        cpu_switch_process();
}


/*
    proc_wake_by_id() - wake up the specified process
*/
void proc_wake_by_id(const pid_t pid)
{
    proc_t *p, *tmp;

    /*
        FIXME: it's not safe to disable/enable interrupts if we're intending to call this fn from
        IRQ handler context.  Trouble is, IRQ handlers need to use this function to wake up
        sleeping processes.  Decisions, decisions.
    */
/*  cpu_disable_interrupts(); */

    list_for_each_entry_safe(p, tmp, &g_sleep_queue, queue)
    {
        if(p->id == pid)
        {
            p->state = ps_runnable;
            list_move_append(&p->queue, &g_run_queue);  /* TODO: should be list_move_insert() */
            break;
        }
    }

    /* FIXME - see above */
/*    cpu_enable_interrupts(); */
}
