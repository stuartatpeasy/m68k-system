/*
	strcat.c - implementation of strcat(), part of string

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-07


	These functions should all be optimised extensively.  It would be worth coding them in assembly
	language, as they are all quite performance-critical.
*/

#include "string.h"


char *strcat(char *dest, const char *src)
{
	char *dest_ = dest + strlen(dest);
	strcpy(dest_, src);

	return dest;
}

