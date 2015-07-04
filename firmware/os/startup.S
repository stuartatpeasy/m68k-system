ram_start = 0x00000400
ram_end   = 0x000ffffc


.text
.org 0

# supervisor stack pointer
.int 0x100000

# reset vector
.int _start

_start:		
#			# zero RAM
#_zeroram:	move.l	(ram_end - ram_start) / 4 - 1, %d0
#			moveq.l	#0, %d1
#			movea	ram_start, %a0
#_zr1:		move.l	%d1, %a0@+
#			dbra	%d0, _zr1

			# branch to program start
			jmp		_main

