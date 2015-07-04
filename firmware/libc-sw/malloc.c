/*
	malloc.c - implementation of malloc(), part of stdlib

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-08-14.
*/

#include "stdlib.h"


void *malloc(size_t size)
{
	return kmalloc(size);	/* FIXME: very bad - shouldn't call this fn directly! */
}

