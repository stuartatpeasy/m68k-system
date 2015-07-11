#ifndef __ASM_RTE_H__
#define __ASM_RTE_H__
/*
	RTE machine instruction "intrinsic"

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, July 2013.
*/

#define rte()	do												\
					{											\
						__asm__ __volatile__					\
						(										\
							"rte"								\
							: 									\
							: 									\
						);										\
					} while(0);									\

#endif

