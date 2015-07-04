/*
	strcmp.c - implementation of strcmp(), part of string

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-07


	These functions should all be optimised extensively.  It would be worth coding them in assembly
	language, as they are all quite performance-critical.
*/

#include "string.h"


int strcmp(const char *s1, const char *s2)
{
	for(; *s1 == *s2; ++s1, ++s2)
		if(*s1 == 0)
			return 0;
	return *(unsigned char *) s1 - *(unsigned char *) s2;
}
