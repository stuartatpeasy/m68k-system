#include "include/syscall.h"

.text

.globl syscall_dispatcher
syscall_dispatcher:
		move.l	%usp, %a0	

		# overwrite TRAP instr's return address on the supervisor stack with the 
		# syscall() return address to save an extra branch
		move.l	%a0@+, %sp@(2)

		# adjust user stack pointer: throw away the syscall() retval, as it won't
		# be used.  The copy on the supervisor stack will be used instead
		move.l	%a0, %usp

		# d0 = system call number
		move.l	%a0@+, %d0

		# place a ptr to the syscall args (if any) on the supervisor stack
		move.l	%a0, %sp@-

		# check that the system call number is valid (<= MAX_SYSCALL); if not, return
		# an error status
		cmpi.l	#MAX_SYSCALL, %d0
		bgt		syscall_handler_invalid_syscall
		
		# calculate the address of the syscall handler and dispatch the call
		movea.l	syscall_table, %a0
		lsl.l	#2, %d0
		add.l	%d0, %a0

		jsr		%a0@

		# retval will be in d0
		rte

syscall_handler_invalid_syscall:
		moveq.l	#-1, %d0
		rte

syscall_table:
		.int 0
		.int 0
		.int 0
		.int 0

