/*
    Functions relating to processes

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/

#include <kernel/process.h>


proc_t *g_current_proc = NULL;
pid_t g_next_pid = 0;


/*
    proc_create() - create a new process and add it to the run queue.
*/
s32 proc_create(const uid_t uid, const gid_t gid, const s8* name, proc_entry_fn_t main_fn, u32 *arg,
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
    proc_destroy() - terminate the current process and clean up.
*/
void proc_destroy()
{

}
