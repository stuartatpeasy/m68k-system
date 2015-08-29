/*
	Application entry point

	Part of libayumos


	Stuart Wallace, August 2015.
*/

.global _start
_start:
		bsr.w main
		move.l %d0, %sp@-
		pea.l 0
		trap #0
