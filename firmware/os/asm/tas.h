#ifndef __ASM_TAS_H__
#define __ASM_TAS_H__
/*
	Test-and-set "intrinsic"

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, December 2012.
*/

#include "include/types.h"


#define tas(addr)								\
	(__extension__ ({							\
		register unsigned int ret;				\
		register unsigned int __addr = (addr);	\
		__asm__ __volatile__					\
		(										\
					"clr.l %[output]\n"			\
					"tas %[address]\n"			\
					"bvc L_%=\n"				\
					"moveq.l #1, %[output]\n"	\
			"L_%=:\n"							\
			: [output] "=&r" (ret)				\
			: [address] "d" (__addr)			\
			: "cc"								\
		);										\
		ret;									\
	}))

#endif

