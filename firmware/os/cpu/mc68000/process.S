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

#include "cpu/mc68000/regs.h"
.include "cpu/mc68000/macros.S"

.text
.global syscall_yield
.global cpu_context_switch

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

    cpu_context_switch is used as the timer interrupt handler.  syscall_yield is the handler for the
    TRAP call used to yield a time-slice.  cpu_context_switch saves CPU context in the proc_t
    structure pointed to by g_current_proc, then calls the sched() function.  That function decides
    which process to run next, and updates g_current_proc accordingly.  cpu_context_switch then
    restores CPU state for the incoming process and resumes execution.

    syscall_yield handles the "yield" system call.  It unwinds the syscall_dispatcher stack frame,
    restoring trashed registers, then falls through to cpu_context_switch.
*/

syscall_yield:
    addq.l      #4, sp
    movem.l     sp@+, d1/a0-a2

    /* fall through */

cpu_context_switch:
    DISABLE_INTERRUPTS
    SAVE_CONTEXT

    jsr         sched       /* Choose next process to run, update g_current_proc accordingly */

    RESTORE_CONTEXT
    ENABLE_INTERRUPTS

    rte


proc_exit:
