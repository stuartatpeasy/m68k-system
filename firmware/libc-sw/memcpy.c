/*
	memcpy.c

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-08-29


	This function should be optimised extensively.  It would be worth coding it in assembly
	language, as it is quite performance-critical.
*/

#include "string.h"


void *memcpy(void *dest, const void *src, size_t n)
{
	char *src_ = (char *) src,
		 *dest_ = (char *) dest;

	if(n > 8)
	{
		size_t n_ = n - 8;
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

