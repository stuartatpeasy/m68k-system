#ifndef KERNEL_SCHED_H_INC
#define KERNEL_SCHED_H_INC
/*
	Scheduler

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, January 2012.
*/

#include <cpu/cpu.h>
#include <include/defs.h>
#include <include/types.h>
#include <strings.h>


typedef void (*proc_main_t)(void *);

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
    Process state structure.  Note that the layout of this struct is very sensitive: in particular,
    struct regs must not be modified, and must be the first item in struct task_struct.

    A movem.l (move multiple regs) instruction is used to fill the d[] and a[] arrays.  The movem.l
    instr only permits pre-decrement addressing when writing registers to memory.  Conversely, when
    transferring memory to registers, only post-increment is allowed.  This means that the "regs"
    struct can use regular arrays, a[8] and d[8], to store register values.
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

    proc_t *next;
    proc_t *prev;
}; /* sizeof(proc_t) = 128 */


proc_t *g_current_proc;
u32 g_ncontext_switches;

void irq_schedule(ku32 irql, void *data, regs_t regs);
s32 sched_init(const char * const init_proc_name);

s32 create_process(const s8 *name, proc_main_t main_fn, u32 *arg, ku32 stack_len, ku16 flags,
                   pid_t *newpid);

#endif
