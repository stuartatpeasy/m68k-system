/*
	strcpy.c - implementation of strcpy(), part of string

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-07


	These functions should all be optimised extensively.  It would be worth coding them in assembly
	language, as they are all quite performance-critical.
*/

#include "string.h"


char *strcpy(char *dest, const char *src)
{
	do
	{
		*dest++ = *src;
	} while(*src++);

	return dest;
}

