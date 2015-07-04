/*
	tolower.c - implementation of tolower(), part of ctype

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-07-01.
*/

#include "ctype.h"


int tolower(int c)
{
	return ((c >= 'A') && (c <= 'Z')) ? c + ('a' - 'A') : c;
}

