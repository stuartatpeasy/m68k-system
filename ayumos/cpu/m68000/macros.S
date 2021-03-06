/*
    Assembler macro "functions"

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/

.set sizeof_regs_t,  74             /* sizeof(regs_t) */

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
    adda.w      #sizeof_regs_t, %a0

    /* Store PC and SR of outgoing process in g_current_proc->regs */
    move.l      %sp@(6), %a0@-
    move.w      %sp@(4), %a0@-

    /*
        Store registers in g_current_proc->regs[].  a0 is trashed at this point, and will be re-read
        from the stack.  Also store the user stack pointer in g_current_proc->regs.usp.
    */
    subq.l      #4, %a0                 /* Step over SP (A7) - it will be saved separately */
    movem.l     %d0-%d7/%a0-%a6, %a0@-
    move.l      %sp@+, %a0@(32)         /* Save original A0, and correct the value of SP */
    move.l      %sp, %a0@(60)           /* Save original SP */
    move.l      %usp, %a1
    move.l      %a1, %a0@-              /* Save USP */
.endm


/*
    Restore CPU context from g_current_proc->regs.  Assume that the top of the stack contains a
    short-form exception frame, and overwrite the values of PC and SR in the exception frame with
    the values in g_current_proc->regs.pc and g_current_proc->regs.sr.

    NOTE: both (user and supervisor) stack pointers are restored.
    NOTE: assumes regs is the first member of g_current_proc.
*/
.macro RESTORE_CONTEXT
    movea.l     g_current_proc, %a0
    move.l      %a0@+, %a1
    move.l      %a1, %usp
    movem.l     %a0@+, %d0-%d7/%a0-%a7
    move.l      %a0@(-32), %a0
.endm


/*
    Save full context of the current process, call sched() to select a new process to run, restore
    the context of the new process.
*/
.macro DO_CONTEXT_SWITCH
    SAVE_CONTEXT

    jsr         sched       /* Choose next process to run, update g_current_proc accordingly */

    RESTORE_CONTEXT
.endm
