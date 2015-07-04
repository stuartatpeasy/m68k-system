/*
	kstrcpy.c - implementation of kstrcpy() [= libc strcpy()]

	Part of the as-yet-unnamed MC68010 operating system

	(c) Stuart Wallace <stuartw@atom.net>, 2012-07

	
	This function doesn't warrant its own module.  The k*() utility functions should
	all be combined into a single library.
*/

#include "include/types.h"
#include "kutil/kutil.h"


s8 *kstrcpy(s8 *dest, ks8 *src)
{
	do
	{
		*dest++ = *src;
	} while(*src++);

	return dest;
}

