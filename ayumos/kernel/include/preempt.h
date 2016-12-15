#ifndef KERNEL_INCLUDE_PREEMPT_H_INC
#define KERNEL_INCLUDE_PREEMPT_H_INC
/*
    Pre-emption enabling/disabling

    Part of ayumos


    (c) Stuart Wallace, December 2016.
*/

#include <kernel/include/cpu.h>
#include <kernel/include/defs.h>
#include <kernel/include/types.h>


extern vu32 preempt_count;


/*
    preempt_disable() - disable CPU interrupts and increment the global pre-emption disable counter.
    This allows nested calls to preempt_disable() / preempt_enable().
*/
inline void preempt_disable()
{
    cpu_disable_interrupts();
    preempt_count++;
}


/*
    preempt_enable() - decrement the global pre-emption disable counter, and enable CPU interrupts
    if the counter has reached zero.  Note that improper nesting of calls to preempt_disable() /
    preempt_enable() will give rise to a race condition on the preempt_count variable.
*/
inline void preempt_enable()
{
    if(--preempt_count)
        cpu_enable_interrupts();
}

#endif
