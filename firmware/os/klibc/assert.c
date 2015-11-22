/*
	assert.c - definition of the assert() function

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, 2011-08-28.
*/

#include <klibc/assert.h>
#include <klibc/stdio.h>		/* for puts(), below */


void assert(s32 expression)
{
	if(!expression)
	{
		puts("assertion failed");

        /* TODO: do something better here, i.e. TRAP back into the kernel */
        while(1) ;
	}
}

