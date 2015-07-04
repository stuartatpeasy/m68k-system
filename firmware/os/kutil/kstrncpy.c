/*
	kstrncpy.c - implementation of kstrncpy() [= libc strncpy()]

	Part of the as-yet-unnamed MC68010 operating system

	(c) Stuart Wallace <stuartw@atom.net>, 2012-07

	
	This function doesn't warrant its own module.  The k*() utility functions should
	all be combined into a single library.
*/

#include "include/types.h"


s8 *kstrncpy(s8 *dest, ks8 *src, u32 n)
{
	u32 n_;

	for(n_ = 0; (n_ < n) && (src[n_] != '\0'); n_++)
		dest[n_] = src[n_];
	
	for(; n_ < n; n_++)
		dest[n_] = '\0';
	
	return dest;
}

