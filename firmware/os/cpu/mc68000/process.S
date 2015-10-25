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

.text
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

    mc68010_context_switch is used as the timer interrupt handler, and the handler for the TRAP
    call used to yield a time-slice.  It saves CPU context in the proc_t structure pointed to by
    g_current_proc, then calls the sched() function.  That function should decide which process to
    run next, and update g_current_proc accordingly before returning.  mc68010_context_switch then
    restores CPU state for the incoming process and resumes execution.
*/


cpu_context_switch:
    ori.w       #0x0700, sr         /* Disable interrupts */

    /* Stack a0 and point it at the first hword after g_current_proc->regs */
    move.l      a0, sp@-

    movea.l     g_current_proc, a0
    adda.w      #70, a0             /* NOTE: sizeof(regs_t) = 70 */

    /* Store PC and SR of outgoing process in g_current_proc->regs */
    move.l      sp@(6), a0@-
    move.w      sp@(4), a0@-

    /*
        Store registers in g_current_proc->regs[].  a0 is trashed at this point, and will be re-read
        from the stack.  Also store the user stack pointer in g_current_proc->regs.a[7].
    */
    movem.l     d0-d7/a0-a7, a0@-
    move.l      sp@+, a0@(32)
    move.l      usp, a1
    move.l      a1, a0@(60)

    jsr         sched

    /*
        Restore state from g_current_proc.
        NOTE: assumes regs is the first member of g_current_proc
    */
    movea.l     g_current_proc, a0
    move.l      a0@(66), sp@(2)
    move.w      a0@(64), sp@
    move.l      a0@(60), a1
    move.l      a1, usp
    movem.l     a0@+, d0-d7/a0-a6
    move.l      a0@(-28), a0

    andi.w      #0xf8ff, sr         /* Enable interrupts */

    rte
