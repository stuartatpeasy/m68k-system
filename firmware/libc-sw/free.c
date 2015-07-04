/*
	free.c - implementation of free(), part of stdlib

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-08-14.
*/

#include "stdlib.h"


void free(const void *ptr)
{
	kfree(ptr);				/* FIXME: very bad! shouldn't call this fn directly */
}

