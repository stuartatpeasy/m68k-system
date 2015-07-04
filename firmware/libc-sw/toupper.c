/*
	toupper.c - implementation of toupper(), part of ctype

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-07-01.
*/

#include "ctype.h"


int toupper(int c)
{
	return ((c >= 'a') && (c <= 'z')) ? c + ('a' - 'A') : c;
}

