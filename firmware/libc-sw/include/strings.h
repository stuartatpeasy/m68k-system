#ifndef __STRINGS_H__
#define __STRINGS_H__
/*
	strings.h - declarations for more string-manipulation functions

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-07-*.
*/

#include "stddef.h"


void bcopy(const void *src, void *dest, size_t n);
void bzero(void *s, size_t n);

#endif

