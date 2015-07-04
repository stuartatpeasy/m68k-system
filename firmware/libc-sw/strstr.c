/*
	strstr.c - implementation of strstr(), part of string

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-07


	These functions should all be optimised extensively.  It would be worth coding them in assembly
	language, as they are all quite performance-critical.
*/

#include "string.h"


char *strstr(const char *haystack, const char *needle)
{
	const char *n;
	for(n = needle; *haystack; ++haystack)
	{
		if(*haystack != *n)
			n = needle;

		if(*haystack == *n)
			if(!*++n)
				return (char *) haystack - strlen(needle) + 1;
	}

	return NULL;
}

