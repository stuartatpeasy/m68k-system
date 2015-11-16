#ifndef KERNEL_PROCESS_H_INC
#define KERNEL_PROCESS_H_INC
/*
    Declarations relating to processes

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/

#include <cpu/cpu.h>
#include <include/defs.h>
#include <include/list.h>
#include <include/types.h>
#include <kernel/user.h>


#define PROC_KSTACK_LEN     (2048)      /* Per-process kernel stack size */

/*
    Process type flags
*/
#define PROC_TYPE_KERNEL    (0x8000)

typedef enum proc_state
{
    ps_unborn = 0,
    ps_runnable,
    ps_sleeping,
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

    pid_t id;
    proc_state_t state;
    s32 exit_code;

    char name[32];
    u32 flags;
    u32 quanta;

    uid_t uid;
    gid_t gid;

    exe_img_t *img;

    void *kstack;   /* ptr to mem alloc'ed for kernel stack, i.e. bottom of stack   */
    void *ustack;   /* ptr to mem alloc'ed for user stack, i.e. bottom of stack     */

    void *arg;

	const proc_t *parent;
	list_t queue;
}; /* sizeof(proc_t) = 80 + sizeof(regs_t) */


s32 proc_create(const uid_t uid, const gid_t gid, const s8 *name, exe_img_t *img,
                proc_entry_fn_t entry, void *arg, ku32 stack_len, ku16 flags,
                const proc_t * const parent, pid_t *newpid);

pid_t proc_get_pid();
proc_t *proc_current();
void proc_do_exit(s32 exit_code);
void proc_sleep();
void proc_wake_by_id(const pid_t pid);

#endif
