/*
    Process-related functions for MC68010 CPU

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.

    NOTE: this code WILL NOT WORK on a MC68000, because the 68000 doesn't store a vector offset in
    its interrupt stack frame.
*/

#ifndef TARGET_MC68010
#error This code requires the MC68010 architecture
#endif

#include <cpu/m68000/regs.h>
.include "cpu/m68000/macros.S"

.text
.global cpu_preempt
.global cpu_switch_process

/*
                 15                     0
                 | ..other junk, maybe..|
                 +------+---------------+
        SP +6    |format| vector offset |
                 +------+---------------+
           +4    | program counter low  |
           +2    | program counter high |
                 +----------------------+
        SP +0    |    status register   |
                 +----------------------+

    "Other junk" refers to additional data stacked for bus and address error exceptions.

    cpu_preempt is used as the timer interrupt handler.  It saves CPU context in the proc_t
    structure pointed to by g_current_proc, then calls the sched() function.  That function decides
    which process to run next, and updates g_current_proc accordingly.  cpu_preempt then restores
    CPU state for the incoming process and resumes execution.
*/

cpu_preempt:
    DISABLE_INTERRUPTS
    DO_CONTEXT_SWITCH
    ENABLE_INTERRUPTS
    rte


/*
    cpu_switch_process switches to a new process context by:
        - converting the stack frame (which contains only the return address for the call to
          cpu_switch_process) into a short-form exception stack frame,
        - switching context using the DO_CONTEXT_SWITCH macro (which modifies the exception stack
          frame),
        - issuing an RTE to begin execution of the incoming process.
*/
cpu_switch_process:
    DISABLE_INTERRUPTS

    move.w      sr, sp@(-4)
    move.l      sp@, sp@(-2)
    clr.w       sp@(2)
    subq.l      #4, sp

    DO_CONTEXT_SWITCH
    ENABLE_INTERRUPTS

    rte
