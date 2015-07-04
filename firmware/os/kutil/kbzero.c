/*
	kbzero.c - implementation of kbzero() [= libc bzero()]

	Part of the as-yet-unnamed MC68010 operating system

	(c) Stuart Wallace <stuartw@atom.net>, 2012-07

	
	This function doesn't warrant its own module.  The k*() utility functions should
	all be combined into a single library.
*/

#include "include/types.h"


void kbzero(void *s, u32 n)
{
	s8 *s_ = s;

	/* Zero individual bytes until *s is word-aligned */
	for(; n && ((u32) s_ & 3); n--)
		*s_++ = 0;

	/* Zero words until n < 4 */
	for(; n & 3; n -= 4, s_ += 4)
		*((u32 * const) s_) = 0;

	/* Zero trailing bytes */
	for(; n; n--, *s_++ = 0) ;
}

