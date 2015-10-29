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
                ku32 stack_len, ku16 flags, pid_t *newpid)
{
    proc_t *p;

    p = CHECKED_KCALLOC(1, sizeof(proc_t));

    u8 *process_stack = (u8 *) umalloc(stack_len);

    u32 *process_stack_top = (u32 *) (process_stack + stack_len);

    p->id = g_next_pid++;
    p->exit_code = S32_MIN;
    p->parent = NULL;
    p->uid = uid;
    p->gid = gid;

    p->img = img;

    strncpy(p->name, name, sizeof(p->name) - 1);
    p->name[sizeof(p->name) - 1] = '\0';

    /* Set up the process's initial stack */
    *(--process_stack_top) = (u32) 0xdeadbeef;  /* proc must exit with syscall, not rts */
    *(--process_stack_top) = (u32) arg;

    cpu_init_proc_regs(&p->regs, img->entry_point, process_stack_top, flags);

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

    /* TODO: deallocate any resources allocated by g_exiting */

    /* TODO: move this code into a generic exe_img_free() fn */
    kfree(g_exiting->img->start);
    kfree(g_exiting->img);

    /* TODO: move g_exiting onto "exited" list, to await exit-code collection? */


    kfree(g_exiting);
}
