/*
	System call executors

	Part of libayumos


	Stuart Wallace, August 2015.
*/

.text
.globl syscall0
.globl syscall1
.globl syscall2
.globl syscall3
.globl sys_yield

syscall0:
		trap #0
		rts

syscall1:
		trap #0
		rts

syscall2:
		trap #0
		rts

syscall3:
		trap #0
		rts

