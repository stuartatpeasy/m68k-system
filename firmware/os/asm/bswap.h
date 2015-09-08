#ifndef ASM_BSWAP_H_INC
#define ASM_BSWAP_H_INC
/*
	Byte-order conversion "intrinsics"

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, December 2011.

	FIXME - these don't work when used on memory operands, e.g. *p = bswap_16(*p).
	For some reason the compiler optimises away the writeback, or something.
*/

#include "include/types.h"

#if defined(TARGET_MC68000) || defined(TARGET_MC68010) \
    || defined(TARGET_MC68020) || defined(TARGET_MC68030)


#define bswap_16(x)											\
					(__extension__ ({						\
						register unsigned short x_ = (x);	\
						__asm__ __volatile__				\
						(									\
							"rol.w #8, %w0"					\
							:       						\
							: "d" (x_)						\
							: "d0", "cc"			        \
						);									\
						x_; 								\
					}))


#define bswap_32(x)											\
					(__extension__ ({					    \
						register unsigned int x_ = (x);	    \
						__asm__ __volatile__				\
						(									\
							"rol.w #8, %0\n"				\
							"swap %0\n"						\
							"rol.w #8, %0"					\
							:                               \
							: "d" (x_)						\
							: "cc"							\
						);									\
						x_; 								\
					}))


#define wswap_32(x)											\
					(__extension__ ({						\
						register unsigned int x_ = (x);	    \
						__asm__ __volatile__				\
						(									\
							"swap %0\n"						\
							:       						\
							: "d" (x_)						\
							: "cc"							\
						);									\
						x_; 								\
					}))

#endif /* defined(TARGET_MC680x0) */
#endif

