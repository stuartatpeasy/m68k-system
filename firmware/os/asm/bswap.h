#ifndef __ASM_BSWAP_H__
#define __ASM_BSWAP_H__
/*
	Byte-order conversion "intrinsics"

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, December 2011.
*/

#include "include/types.h"


#define __bswap_16(x)											\
					(__extension__ ({							\
						register unsigned short __x = (x);		\
						__asm__ __volatile__					\
						(										\
							"rol.w #8, %w0"						\
							: 									\
							: "d" (__x)							\
							: "d0", "cc"						\
						);										\
						__x;									\
					}))


#define __bswap_32(x)											\
					(__extension__ ({							\
						register unsigned int __x = (x);		\
						__asm__ __volatile__					\
						(										\
							"rol.w #8, %0\n"					\
							"swap %0\n"							\
							"rol.w #8, %0"						\
							: 									\
							: "d" (__x)							\
							: "cc"								\
						);										\
						__x;									\
					}))


#define __wswap_32(x)											\
					(__extension__ ({							\
						register unsigned int __x = (x);		\
						__asm__ __volatile__					\
						(										\
							"swap %0\n"							\
							: 									\
							: "d" (__x)							\
							: "cc"								\
						);										\
						__x;									\
					}))

#endif

