;#include "kernel/syscall.h"

# FIXME
#define MAX_SYSCALL 3

.text

/*
    On entry, the user stack will look like this:

        |   other stuff     |
        +-------------------+
        | [syscall argument |
        |         3]        |   <-- USP + 16
        +-------------------+
        | [syscall argument |
        |         2]        |   <-- USP + 12
        +-------------------+
        |  syscall argument |
        |         1         |   <-- USP + 8
        +-------------------+
        | requested syscall |
        |      number       |   <-- USP + 4
        +-------------------+
        |   return address  |
        |                   |   <-- USP + 0
        +-------------------+

*/

.globl syscall_dispatcher
syscall_dispatcher:
        movem.l     %d1/%a0-%a2, %sp@-
		move.l      %usp, %a0

        # find out which syscall was requested
        move.l      %a0@(4), %d0

        # verify that the requested syscall exists
        cmpi.l      #MAX_SYSCALL, %d0
        bgt         syscall_handler_invalid_syscall

        # calculate address of entry in syscall_table corresponding to the requested call
        # = syscall_table + (syscall_num * sizeof(syscall_table_entry))
        # = syscall_table + (syscall_num * 8)
        movea.l     syscall_table, %a1
        lsl.l       #3, %d0
        adda.l      %d0, %a1

        # d0 <-- num of args needed by syscall
        move.l      %a1@, %d0

        # copy call args to supervisor stack
        # skip this process if the call takes no args
        movea.l     %sp, %a2
        beq.s       sdL2

        # calculate address of first arg on user stack
        # = usp + sizeof(return_addr) + sizeof(syscall_num) + (num_args * sizeof(arg))
        # = usp + 8 + (num_args * 4)
        move.l      %d0, %d1
        lsl.l       #2, %d1
        addq.l      #8, %d1
        adda.l      %d0, %a0

        # push args to supervisor stack
sdL1:   move.l      %a0@-, %sp@-
        dbf.w       %d0, sdL1

        # do syscall
sdL2:   jsr         %a1@(4)

        # deallocate syscall args and restore clobbered regs
        movea.l     %a2, %sp
        movem.l     %sp@+, %d1/%a0-%a2

        # syscall retval will be in d0
        rte

syscall_handler_invalid_syscall:
		moveq.l	#-1, %d0
		rte

/*
    System call pointer table

    Format:

        int32:  # args needed
        int32:  ptr to handler function
*/
.section .rodata
.align 4
syscall_table:
		.int 1
		.int syscall_exit
		.int 1
		.int syscall_console_putchar
		.int 0
		.int syscall_console_getchar
		.int 1
		.int syscall_leds

