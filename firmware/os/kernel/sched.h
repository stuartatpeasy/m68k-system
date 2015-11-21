#ifndef KERNEL_SCHED_H_INC
#define KERNEL_SCHED_H_INC
/*
	Scheduler

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, January 2012.
*/

#include <include/defs.h>
#include <include/types.h>
#include <kernel/cpu.h>
#include <kernel/process.h>
#include <kernel/user.h>
#include <strings.h>


u32 g_ncontext_switches;
extern proc_t *g_current_proc;
extern list_t g_sleep_queue;
extern list_t g_run_queue;

void sched();
s32 sched_init(const char * const init_proc_name);

#endif
