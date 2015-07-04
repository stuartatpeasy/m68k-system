#ifndef __SCHED_SCHED_H__
#define __SCHED_SCHED_H__
/*
	Scheduler

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, January 2012.
*/

#include "include/types.h"

/*
	SCHED_DISABLE() -	disable scheduler by ensuring that IRQ mask is >= 1.
						Return original IRQ mask level.

	TODO: instead take this approach:
		1. read IRQ mask level
		2. if level == 0, set level to 1
		3. disable the context-switch timer
		4. restore the mask level read in the first step
*/
#define SCHED_DISABLE()										\
					(__extension__ ({						\
						register unsigned short __m = 0;	\
						__asm__ __volatile__				\
						(									\
							"move.w	%%sr, %w0\n"			\
							"andi.w #0x0700, %w0\n"			\
							"bne.s .Lsd_fini\n"				\
							"ori.w	#0x0100, %%sr\n"		\
							".Lsd_fini:"					\
							: "=d"(__m)						\
							: /* no inputs */				\
							: "cc"							\
						);									\
						__m;								\
					}))


/*
	SCHED_ENABLE(x) - 	enable scheduler by restoring the IRQ mask level saved by
						an invocation of SCHED_DISABLE()

	TODO: instead take this approach:
		1. ???
*/
#define SCHED_ENABLE(x)	\
					(__extension__ ({	\
						register unsigned short __m = (x);	\
						__asm__ __volatile__				\
						(									\
							"move.w %w0, %%sr\n"			\
							: /* no outputs */				\
							: "d" (__m)						\
							: "cc"							\
						);									\
					}))



typedef u32 reg_t;

struct task_struct
{
	reg_t d[8];
	reg_t a[8];

	reg_t sr;
	reg_t pc;

	struct task_struct *next;
};

typedef struct task_struct task_t;

volatile task_t *g_current_task;


#endif

