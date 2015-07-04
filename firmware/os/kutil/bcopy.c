/*
	bcopy.c

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-08-29


	This function should be optimised extensively.  It would be worth coding it in assembly
	language, as it is quite performance-critical.
*/

#include "include/types.h"


void bcopy(const void *src, void *dest, u32 n)
{
	const char *src_ = src;
	char *dest_ = dest;

	if(dest > src)
	{
		/* copy backwards */
		while(n--)
			dest_[n] = src_[n];
	}
	else if(dest < src)
	{
		/* copy forwards */
		while(n--)
			*dest_++ = *src_++;
	}
}

