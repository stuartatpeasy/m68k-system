#ifndef __ASM_STOP_H__
#define __ASM_STOP_H__
/*
	STOP machine instruction "intrinsic"

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, December 2011.
*/

#define stop(x)	do											\
					{										\
						__asm__ __volatile__				\
						(									\
							"stop %w0"						\
							: 								\
							: "n" (x)						\
						);									\
					} while(0);								\

#endif

