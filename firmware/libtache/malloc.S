.text

.globl os_malloc
os_malloc:
			moveq.l	#0, %d0
			move.l	%sp@(8), %d1
			trap	#1
			rts

