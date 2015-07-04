#ifndef __ASM_RESET_H__
#define __ASM_RESET_H__
/*
	"intrinsic" which hard-resets the hardware and the CPU

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, February 2012.
*/

#define __sys_reset()	do									\
						{									\
							__asm__ __volatile__			\
							(								\
								"reset\n"					\
								"lea.l	0x00f00000, %%a0\n"	\
								"move.l	%%a0@, %%a7\n"		\
								"addq.l	#4, %%a0\n"			\
								"move.l	%%a0@, %%a0\n"		\
								"jmp	%%a0@\n"			\
								: :							\
							);								\
						} while(0);							\

#endif

