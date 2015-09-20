#ifndef SCHED_SCHED_H_INC
#define SCHED_SCHED_H_INC
/*
	Scheduler

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, January 2012.
*/

#include "cpu/exceptions.h"
#include "cpu/utilities.h"
#include "device/duart.h"          /* DUART generates the scheduler interrupt */
#include "include/defs.h"
#include "kernel/syscall.h"
#include "include/types.h"
#include <strings.h>


typedef u32 reg32_t;
typedef u16 reg16_t;

typedef void *(*proc_main_t)(void *);

/*
    Process type flags
*/
#define PROC_TYPE_KERNEL    (0x8000)

enum proc_state
{
    ps_unborn = 0,
    ps_runnable,
    ps_exited
};

/*
    Process state structure.  Note that the layout of this struct is very sensitive: in particular,
    struct regs must not be modified, and must be the first item in struct task_struct.

    A movem.l (move multiple regs) instruction is used to fill the d[] and a[] arrays.  The movem.l
    instr only permits pre-decrement addressing when writing registers to memory.  Conversely, when
    transferring memory to registers, only post-increment is allowed.  This means that the "regs"
    struct can use regular arrays, a[8] and d[8], to store register values.
*/
struct proc_struct
{
    struct
    {
        reg32_t d[8];
        reg32_t a[8];
        reg32_t pc;
        reg16_t sr;
    } regs;

	const struct proc_struct *parent;       /* needed yet? */
	struct proc_struct *next;               /* needed yet? */

    pid_t id;
    enum proc_state state;

    u8 name[32];
};  /* sizeof(struct proc_struct) = 116 bytes */

typedef struct proc_struct proc_t;

proc_t *g_current_proc;
u32 g_ncontext_switches;

IRQ_HANDLER_DECL(irq_schedule);
void sched_init(void);

pid_t create_process(const s8 *name, proc_main_t main_fn, u32 *arg, ku32 stack_len, ku16 flags);

void sched_dump_proc(proc_t *ps);

#endif
