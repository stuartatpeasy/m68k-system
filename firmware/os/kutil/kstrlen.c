/*
	kstrlen.c - implementation of kstrlen() [= libc strlen()]

	Part of the as-yet-unnamed MC68010 operating system

	(c) Stuart Wallace <stuartw@atom.net>, 2012-07

	
	This function doesn't warrant its own module.  The k*() utility functions should
	all be combined into a single library.
*/

#include "include/types.h"


u32 kstrlen(const s8 *s)
{
	const s8 *s_ = s;
	for(s_ = s; *s; ++s) ;
	return s - s_;
}

