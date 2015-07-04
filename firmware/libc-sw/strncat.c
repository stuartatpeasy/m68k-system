/*
	strncat.c - implementation of strncat(), part of string

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-07


	These functions should all be optimised extensively.  It would be worth coding them in assembly
	language, as they are all quite performance-critical.
*/

#include "string.h"


char *strncat(char *dest, const char *src, size_t n)
{
	size_t n_, dest_len = strlen(dest);

	for(n_ = 0; (n_ < n) && (src[n_] != '\0'); n_++)
		dest[dest_len + n_] = src[n_];
	
	dest[dest_len + n_] = '\0';

	return dest;
}

