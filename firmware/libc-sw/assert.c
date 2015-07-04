/*
	assert.c - definition of the assert() function

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-08-28.
*/

#include "assert.h"

#include "stdio.h"		/* for puts(), below */

void assert(int expression)
{
	if(!expression)
	{
		puts("assertion failed");
		/* FIXME: abort execution here! */
		while(1) ;
	}
}

