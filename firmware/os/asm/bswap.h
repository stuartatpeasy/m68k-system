#ifndef ASM_BSWAP_H_INC
#define ASM_BSWAP_H_INC
/*
	Byte-order conversion "intrinsics"

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, December 2011.
*/

#include "include/types.h"


#define bswap_16(x)											\
					(__extension__ ({						\
						register unsigned short x_ = (x);	\
						__asm__ __volatile__				\
						(									\
							"rol.w #8, %w0"					\
							: 								\
							: "d" (x_)						\
							: "d0", "cc"					\
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
							: 								\
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
							: 								\
							: "d" (x_)						\
							: "cc"							\
						);									\
						x_; 								\
					}))

#endif

