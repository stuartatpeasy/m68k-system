/*
	kputs.c: definition of the kputs() function, used in kernel debugging

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, May 2012.
*/

#include "include/debug.h"
#include "include/defs.h"
#include "duart.h"

#ifdef TEST
#include <stdio.h>
#endif


s32 kputs(ks8 *s)
{
#ifndef TEST
	while(*s)
		duarta_putc(*s++);
	duarta_putc('\n');

	return SUCCESS;
#else
	return puts(s);
#endif
}

