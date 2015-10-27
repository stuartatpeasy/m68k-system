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

.include "cpu/mc68000/regs.h"
.include "cpu/mc68000/macros.S"

.text
.global cpu_context_switch
.global cpu_yield

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

    mc68010_context_switch is used as the timer interrupt handler, and the handler for the TRAP
    call used to yield a time-slice.  It saves CPU context in the proc_t structure pointed to by
    g_current_proc, then calls the sched() function.  That function should decide which process to
    run next, and update g_current_proc accordingly before returning.  mc68010_context_switch then
    restores CPU state for the incoming process and resumes execution.
*/

cpu_context_switch:
cpu_yield:
    DISABLE_INTERRUPTS
    SAVE_CONTEXT

    jsr         sched

    RESTORE_CONTEXT
    ENABLE_INTERRUPTS

    rte


proc_exit:
