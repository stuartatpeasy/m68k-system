/*
	atol.c - implementation of atol(), part of stdlib

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-09-03.

	atoi() and atol() are identical in all respects except for the type of the return value.
	This code assumes that ints are the same size as longs on the target platform.
*/

#include "stdlib.h"


long atol(const char *nptr)
{
	long n = 0;
	unsigned char digit = 0, negative = 0;

	while((*nptr == ' ') || (*nptr == '\t') || (*nptr == '\r') || (*nptr == '\n'))
		++nptr;

	if(*nptr == '-')
	{
		negative = 1;
		++nptr;
	}
	else if(*nptr == '+')
		++nptr;

	while((*nptr >= '0') && (*nptr <= '9') && (digit++ < 10))
		n = (n << 3) + (n << 1) + (*nptr++ - '0');

	return negative ? ~n + 1 : n;
}

