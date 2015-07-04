/*
	kstrcmp.c - implementation of kstrcmp() [= libc strcmp()]

	Part of the as-yet-unnamed MC68010 operating system

	(c) Stuart Wallace <stuartw@atom.net>, 2012-07

	
	This function doesn't warrant its own module.  The k*() utility functions should
	all be combined into a single library.
*/

#include "include/types.h"


s32 kstrcmp(ks8 *s1, ks8 *s2)
{
	for(; *s1 == *s2; ++s1, ++s2)
		if(*s1 == 0)
			return 0;
	return *(u8 *) s1 - *(u8 *) s2;
}
