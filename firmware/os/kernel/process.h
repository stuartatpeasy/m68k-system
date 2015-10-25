#ifndef KERNEL_PROCESS_H_INC
#define KERNEL_PROCESS_H_INC
/*
    Declarations relating to processes

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/

#include <cpu/cpu.h>
#include <include/defs.h>
#include <include/types.h>
#include <kernel/user.h>


/*
    Process type flags
*/
#define PROC_TYPE_KERNEL    (0x8000)

typedef enum proc_state
{
    ps_unborn = 0,
    ps_runnable,
    ps_exited
} proc_state_t;

/*
    Process state structure.  Note that the layout of this struct is sensitive: in particular,
    struct regs must be the first item in struct task_struct.
*/
typedef struct proc_struct proc_t;

struct proc_struct
{
    regs_t regs;

	const struct proc_struct *parent;       /* needed yet? */

    pid_t id;
    proc_state_t state;

    char name[32];
    u32 flags;
    u32 quanta;

    uid_t uid;
    gid_t gid;

    proc_t *next;
    proc_t *prev;
};


s32 proc_create(const uid_t uid, const gid_t gid, const s8 *name, proc_entry_fn_t main_fn,
                   u32 *arg, ku32 stack_len, ku16 flags, pid_t *newpid);

pid_t proc_get_pid();


#endif
