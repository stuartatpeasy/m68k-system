.text

.globl os_free
os_free:
			moveq.l	#1, %d0
			trap	#1
			rts

