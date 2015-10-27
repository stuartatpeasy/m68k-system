/*
    Assembler macro "functions"

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/


/*
    Disable interrupts
*/
.macro DISABLE_INTERRUPTS
    ori.w       #0x0700, %sr        /* Disable interrupts */
.endm


/*
    Enable interrupts
*/
.macro ENABLE_INTERRUPTS
    andi.w      #0xf8ff, %sr        /* Enable interrupts */
.endm


/*
    Save complete CPU context in g_current_proc->regs.

    Trashes a0 and a1.
*/
.macro SAVE_CONTEXT
    /* Stack a0 and point it at the first hword after g_current_proc->regs */
    move.l      %a0, %sp@-

    movea.l     g_current_proc, %a0
    adda.w      #74, %a0            /* NOTE: sizeof(regs_t) = 74 */

    /* Store PC and SR of outgoing process in g_current_proc->regs */
    move.l      %sp@(6), %a0@-
    move.w      %sp@(4), %a0@-
    subq.l      #4, %a0

    /*
        Store registers in g_current_proc->regs[].  a0 is trashed at this point, and will be re-read
        from the stack.  Also store the user stack pointer in g_current_proc->regs.usp.
    */
    movem.l     %d0-%d7/%a0-%a7, %a0@-
    move.l      %sp@+, %a0@(32)
    move.l      %usp, %a1
    move.l      %a1, %a0@(64)
.endm


/*
    Restore state from g_current_proc->regs.  Assume that the top of the stack contains a short-form
    exception frame, and overwrite the values of PC and SR in the exception frame with the values in
    g_current_proc->regs.pc and g_current_proc->regs.sr.  The master stack pointer is not restored.

    NOTE: assumes regs is the first member of g_current_proc.
*/
.macro RESTORE_CONTEXT
    movea.l     g_current_proc, %a0
    move.l      %a0@(70), %sp@(2)
    move.w      %a0@(68), %sp@
    move.l      %a0@(64), %a1
    move.l      %a1, %usp
    movem.l     %a0@+, %d0-%d7/%a0-%a6
    move.l      %a0@(-28), %a0
.endm
