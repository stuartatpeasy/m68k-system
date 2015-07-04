/*
	memchr.c - implementation of memchr(), part of string

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-07


	These functions should all be optimised extensively.  It would be worth coding them in assembly
	language, as they are all quite performance-critical.
*/

#include "string.h"


void *memchr(const void *s, int c, size_t n)
{
	const unsigned char *s_ = s;
	for(; n--; ++s_)
		if(*s_ == (unsigned char) c)
			return (void *) s_;
	
	return NULL;
}

