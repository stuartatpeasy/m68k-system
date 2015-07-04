/*
	kstrstr.c - implementation of kstrstr() [= libc strstr()]

	Part of the as-yet-unnamed MC68010 operating system

	(c) Stuart Wallace <stuartw@atom.net>, 2013-01



	This function doesn't warrant its own module.  The k*() utility functions should
	all be combined into a single library.
*/

#include "include/types.h"
#include "kutil/kutil.h"


s8 *kstrstr(ks8 *haystack, ks8 *needle)
{
	ks8 *n;
	for(n = needle; *haystack; ++haystack)
	{
		if(*haystack != *n)
			n = needle;

		if(*haystack == *n)
			if(!*++n)
				return (s8 *) haystack - kstrlen(needle) + 1;
	}

	return NULL;
}

