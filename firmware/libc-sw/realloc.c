/*
	realloc.c - implementation of realloc(), part of stdlib

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-08-14.
*/

#include "stdlib.h"


void *realloc(void *ptr, size_t size)
{
	return krealloc(ptr, size);		/* FIXME: very bad! shouldn't call this fn directly */
}

