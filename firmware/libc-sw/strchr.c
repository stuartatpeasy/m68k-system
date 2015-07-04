/*
	strchr.c - implementation of strchr(), part of string

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-07


	These functions should all be optimised extensively.  It would be worth coding them in assembly
	language, as they are all quite performance-critical.
*/

#include "string.h"


char *strchr(const char *s, int c)
{
	for(; *s; ++s)
		if(*s == (char) c)
			return (char *) s;
	
	return NULL;
}

