/*
	strdup.c - implementation of strdup(), part of string

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-07


	These functions should all be optimised extensively.  It would be worth coding them in assembly
	language, as they are all quite performance-critical.
*/

#include "string.h"

#include "errno.h"
#include "stdlib.h"


char *strdup(const char *s)
{
	const size_t len = strlen(s);
	char *buf = malloc(len + 1);
	
	if(buf == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}
	
	strcpy(buf, s);

	return buf;
}

