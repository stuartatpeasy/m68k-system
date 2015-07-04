/*
	kput.c: definition of the kput() function, used in kernel debugging
	Like puts() but doesn't write a terminating \0.

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, May 2012.
*/

#include "include/defs.h"
#include "duart.h"


s32 kput(ks8 *s)
{
	while(*s)
		duarta_putc(*s++);

	return SUCCESS;
}

