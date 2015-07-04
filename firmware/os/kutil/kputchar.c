/*
	kputchar.c - implementation of kputchar() [= libc putchar()]

	Part of the as-yet-unnamed MC68010 operating system

	(c) Stuart Wallace <stuartw@atom.net>, 2012-07

	
	This function doesn't warrant its own module.  The k*() utility functions should
	all be combined into a single library.
*/

#include "include/types.h"
#include "duart.h"


s32 kputchar(s32 c)
{
	duarta_putc(c);

	return c;
}

