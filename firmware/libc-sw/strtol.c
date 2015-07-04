/*
	strtol.c - implementation of strtol(), part of stdlib

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-09-03.
*/

#include "errno.h"
#include "limits.h"
#include "stdlib.h"


long strtol(const char *nptr, char **endptr, int base)
{
	unsigned char neg = 0;
	unsigned long n;

	while((*nptr == ' ') || (*nptr == '\t') || (*nptr == '\r') || (*nptr == '\n'))
		++nptr;
	
	if(*nptr == '-')
	{
		neg = 1;
		++nptr;
	}
	else if(*nptr == '+')
		++nptr;

	n = strtoul(nptr, endptr, base);

	return (n & 0x80000000) ? (neg ? LONG_MIN : LONG_MAX) :
							  (neg ? -((long) n) : (long) n);
}

