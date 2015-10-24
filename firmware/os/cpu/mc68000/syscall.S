/*
    System call dispatcher

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/

#include <cpu/mc68000/regs.h>
#include <kernel/syscalls.h>

.text

/*
    On entry, the user stack will look like this:

                15                0
                |   other stuff   |
                +-----------------+
                | [syscall arg 3] |
           +16  |                 |
                +-----------------+
                | [syscall arg 2] |
           +12  |                 |
                +-----------------+
                |  syscall arg 1  |
            +8  |                 |
                +-----------------+
                |      syscall    |
            +4  |        num      |
                +-----------------+
                |  return address |
        USP +0  |                 |
                +-----------------+
*/

.globl syscall_dispatcher
syscall_dispatcher:
        movem.l     d1/a0-a2, sp@-
		move.l      usp, a0

        /* Retrieve the requested syscall number from the stack, and check that it is valid */
        move.l      a0@(4), d0
        cmpi.l      #MAX_SYSCALL, d0
        bgt         syscall_handler_invalid_syscall

        /* Calculate the offset of the call entry in syscall_table */
        movea.l     g_syscalls, a1
        lsl.l       #3, d0
        adda.l      d0, a1

        /* At this point, d0 contains the number of args needed by the requested syscall */
        move.l      a1@, d0

        /* If the call requires arguments, copy them to the supervisor stack */
        movea.l     sp, a2
        beq.s       sdL2        /* Call takes no args */

        /*
            Calculate the address of the first argument on the user stack;
                = usp + sizeof(return_addr) + sizeof(syscall_num) + (num_args * sizeof(arg))
                = usp + 8 + (num_args * 4)
        */
        move.l      d0, d1
        lsl.l       #2, d1
        addq.l      #8, d1
        adda.l      d0, a0

        /* Push args to supervisor stack */
sdL1:   move.l      a0@-, sp@-
        dbf.w       d0, sdL1

        /* Do syscall */
sdL2:   movel       a1@(4), a1
        jsr         a1@

        /* Deallocate syscall args and restore regs */
        movea.l     a2, sp
        movem.l     sp@+, d1/a0-a2

        /* syscall retval will be in d0 */
        rte

syscall_handler_invalid_syscall:
		moveq.l	#-1, d0
		rte
