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

#include <cpu/mc68000/regs.h>
.include "cpu/mc68000/macros.S"

.text
.global syscall_yield
.global syscall_exit
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

    cpu_preempt is used as the timer interrupt handler.  syscall_yield is the handler for the TRAP
    call used to yield a time-slice.  cpu_preempt saves CPU context in the proc_t structure pointed
    to by g_current_proc, then calls the sched() function.  That function decides which process to
    run next, and updates g_current_proc accordingly.  cpu_preempt then restores CPU state for the
    incoming process and resumes execution.

    syscall_yield handles the "yield" system call.  It unwinds the syscall_dispatcher stack frame,
    restoring trashed registers, then falls through to cpu_preempt.
*/

syscall_yield:
    addq.l      #4, sp
    movem.l     sp@+, d1/a0-a2

    /* fall through */

cpu_preempt:
    DO_CONTEXT_SWITCH

    rte


syscall_exit:
    DISABLE_INTERRUPTS

    /*
        The syscall_dispatcher stack frame consists of four saved registers d1/a0-a2 (16 bytes),
        the one argument to syscall_exit (4 bytes) and the return address for the jsr used to invoke
        the syscall handler (4 bytes).  The total stack frame size is therefore 24 bytes.
    */
    adda.w      #24, sp             /* Unwind the syscall_dispatcher stack frame */
    move.l      sp@(-20), sp@       /* Reposition exit() arg immediately below exception frame */

    jsr         proc_do_exit

    addq.l      #4, sp

    RESTORE_CONTEXT
    ENABLE_INTERRUPTS

    rte


cpu_switch_process:
    move.w      sr, sp@(-4)
    subq.l      #2, sp
    move.l      sp@(2), sp@
    subq.l      #2, sp
    clr.w       sp@(6)

    DO_CONTEXT_SWITCH

    rte
