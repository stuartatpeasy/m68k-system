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

.global mc68010_irq_sched
.global mc68010_sched_yield

/*
    stack(a0)

    a0 = &g_current_proc->regs;


    unstack(a0)


    On entry, the stack will look like this:

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



*/


mc68010_irq_sched:
    /* Stack a0 and point it at the first hword after g_current_proc->regs */
    movel       %a0, %sp@-
    lea         g_current_proc, %a0
    moveal      %a0@, %a0

    addaw       #70, %a0            /* NOTE: sizeof(regs_t) = 70 */

    /* Store PC and SR of outgoing process in g_current_proc->regs */
    movel       %sp@(6), %a0@-
    movew       %sp@(4), %a0@-

    /*
        Store registers in g_current_proc->regs[].  a0 is trashed at this point, and will be re-read
        from the stack.  Also store the user stack pointer in g_current_proc->regs.a[7].
    */
    moveml      %d0-%d7/%a0-%a7, %a0@-
    movel       %sp@+, %a0@(32)
    movel       %usp, %a1
    movel       %a1, %a0@(60)

    jsr         irq_schedule

    /*
        Restore state from g_current_proc.
        NOTE: assumes regs is the first member of g_current_proc
    */
    lea         g_current_proc, %a0
    moveal      %a0@, %a0

    movel       %a0@(66), %sp@(2)
    movew       %a0@(64), %sp@
    movel       %a0@(60), %a1
    movel       %a1, %usp
    moveml      %a0@+, %d0-%d7/%a0-%a6
    movel       %a0@(-28), %a0

    rte
