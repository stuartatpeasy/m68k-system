/*
	strlen.c - implementation of strlen(), part of string

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-07


	These functions should all be optimised extensively.  It would be worth coding them in assembly
	language, as they are all quite performance-critical.
*/

#include "string.h"


size_t strlen(const char *s)
{
	const char *s_ = s;
	for(s_ = s; *s; ++s) ;
	return s - s_;
}

