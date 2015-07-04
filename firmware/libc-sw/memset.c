/*
	memset.c - implementation of memset(), part of string

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-07


	These functions should all be optimised extensively.  It would be worth coding them in assembly
	language, as they are all quite performance-critical.
*/

#include "string.h"


void *memset(const void *s, int c, size_t n)
{
	unsigned char * s_ = (unsigned char *) s;
	for(; n--;)
		*s_++ = c;
	
	return (void *) s;
}

