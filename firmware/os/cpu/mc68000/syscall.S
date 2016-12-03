/*
    System call dispatcher

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/

#include <cpu/mc68000/regs.h>
#include <kernel/include/syscalls.h>
#include <klibc/errors.h>
.include "cpu/mc68000/macros.S"

.text
.global syscall_dispatcher
.global syscall_exit
.global syscall_yield

/*
    On entry, the stacks will look like this:

                      USER STACK                       SUPERVISOR STACK

                15                     0
                |     other stuff      |
                +----------------------+
                |   [ syscall arg ]    |
           +16  |   [      3      ]    |
                +----------------------+
                |   [ syscall arg ]    |
           +12  |   [      2      ]    |
                +----------------------+           15                     0
                |   [ syscall arg ]    |           | ..other junk, maybe..|
            +8  |   [      1      ]    |           +----------------------+
                +----------------------+       +6  |format| vector offset |
                |       syscall        |           +----------------------+
            +4  |         num          |           |    program counter   |
                +----------------------+       +2  |                      |
                |       return         |           +----------------------+
       USP  +0  |       address        |    SP +0  |    status register   |
                +----------------------+           +----------------------+

    The syscall_dispatcher function validates the requested syscall number, copies the call args to
    the supervisor stack, and invokes the syscall handler.  When the handler returns,
    syscall_dispatcher unwinds its stack frame and returns to the caller.  The syscall return value
    is left in d0.
*/

syscall_dispatcher:
        movem.l     d1/a0-a2, sp@-
		move.l      usp, a0

        /* Retrieve the requested syscall number from the stack, and check that it is valid */
        move.l      a0@(4), d0
        cmpi.l      #MAX_SYSCALL, d0
        ble         sd0
        clr.l       d0          /* Invalid syscall num: force it to 0 (=syscall_invalid) */

        /* Calculate the offset of the call entry in syscall_table */
sd0:    lea.l       g_syscalls, a1
        lsl.l       #3, d0
        adda.l      d0, a1

        /* Set d0 = num args required by syscall */
        move.l      a1@, d0

        /* If the call requires arguments, copy them to the supervisor stack */
        movea.l     sp, a2
        beq.s       sdL2        /* Call takes no args */

        /*
            Calculate the address of the first argument on the user stack:
                = usp + sizeof(return_addr) + sizeof(syscall_num) + (num_args * sizeof(arg))
                = usp + 8 + (num_args * 4)
        */
        move.l      d0, d1
        lsl.l       #2, d1
        addq.l      #8, d1
        adda.l      d1, a0
        subq.l      #1, d0

        /* Push args to supervisor stack */
sdL1:   move.l      a0@-, sp@-
        dbf.w       d0, sdL1

        /* Do syscall */
sdL2:   move.l      a1@(4), a1
        jsr         a1@

        /* Deallocate syscall args and restore regs */
sdL3:   movea.l     a2, sp
        movem.l     sp@+, d1/a0-a2

        /* syscall retval will be in d0 */
        rte


/*
    syscall_yield unwinds the syscall_dispatcher stack frame and performs a context switch.  When
    control returns to syscall yield (at the start of the next time-slice for the yielding process)
    syscall_yield returns 0.  It is necessary to unwind the syscall_dispatcher stack frame because
    DO_CONTEXT_SWITCH requires a short-form exception stack frame to be at the top of the stack.
*/
syscall_yield:
    DISABLE_INTERRUPTS
    addq.l      #4, sp
    movem.l     sp@+, d1/a0-a2
    clr.l       d0              /* syscall_yield() always returns 0 */

    DO_CONTEXT_SWITCH
    ENABLE_INTERRUPTS
    rte


/*

*/
syscall_exit:
    DISABLE_INTERRUPTS

    /*
        The syscall_dispatcher stack frame consists of four saved registers d1/a0-a2 (16 bytes),
        the one argument to syscall_exit (4 bytes) and the return address for the jsr used to invoke
        the syscall handler (4 bytes).  The total stack frame size is therefore 24 bytes.
    */
    adda.w      #24, sp             /* Unwind the syscall_dispatcher stack frame */
    move.l      sp@(-20), sp@       /* Reposition exit() arg immediately below exception frame */

    jsr         proc_destroy

    addq.l      #4, sp

    RESTORE_CONTEXT
    ENABLE_INTERRUPTS

    rte
