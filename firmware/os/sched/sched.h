#ifndef __SCHED_SCHED_H__
#define __SCHED_SCHED_H__
/*
	Scheduler

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, January 2012.
*/

#include "duart.h"          /* DUART generates the scheduler interrupt */
#include "cpu/utilities.h"
#include "include/types.h"


typedef u32 reg32_t;
typedef u16 reg16_t;

/*
    Task state structure.  Note that the layout of this struct is very sensitive: the order in
    which the registers appear *must not be changed*.

    A movem.l (move multiple regs) instruction is used to fill the d[] and a[] arrays.  The movem.l
    instr only permits pre-decrement addressing when writing registers to memory.  Conversely, when
    transferring memory to registers, only post-increment is allowed.  This means that the "regs"
    struct can use regular arrays, a[8] and d[8], to store register values.
*/
struct task_struct
{
    struct
    {
        reg32_t d[8];
        reg32_t a[8];
        reg32_t pc;
        reg16_t sr;
    } regs;

	const struct task_struct *parent;
	struct task_struct *next;
	const u8 *name;
};

typedef struct task_struct task_t;

volatile task_t *g_current_task;

void irq_schedule(void) __attribute__((interrupt_handler));
void sched_init(void);

#endif

