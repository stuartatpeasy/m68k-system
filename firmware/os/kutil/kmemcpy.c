/*
	kmemcpy.c - implementation of kmemcpy() [= libc memcpy()]

	Part of the as-yet-unnamed MC68010 operating system

	(c) Stuart Wallace <stuartw@atom.net>, 2012-07

	
	This function doesn't warrant its own module.  The k*() utility functions should
	all be combined into a single library.
*/

#include "include/types.h"

void *kmemcpy(void *dest, const void *src, ku32 n)
{
	s8 *src_ = (s8 *) src,
		 *dest_ = (s8 *) dest;

	if(n > 8)
	{
		u32 n_ = n - 8;
		for(; n_--; *dest_++ = *src_++) ;
	}

	if(n)
	{
		switch(n)
		{
			case 8:		*dest_++ = *src_++;
			case 7:		*dest_++ = *src_++;
			case 6:		*dest_++ = *src_++;
			case 5:		*dest_++ = *src_++;
			case 4:		*dest_++ = *src_++;
			case 3:		*dest_++ = *src_++;
			case 2:		*dest_++ = *src_++;
			case 1:		*dest_++ = *src_++;
			case 0:
			default:
				break;
		}
	}

	return dest;
}

