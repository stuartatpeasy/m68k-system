/*
	calloc.c - implementation of calloc(), part of stdlib

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-08-14.
*/

#include "stdlib.h"


void *calloc(size_t nmemb, size_t size)
{
	return kcalloc(nmemb, size);		/* FIXME: very bad! shouldn't call this fn directly! */
}

