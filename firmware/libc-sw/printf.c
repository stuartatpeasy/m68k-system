/*
	printf.c - implementation of printf(), part of stdio

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-08
*/

#include "stdio.h"


int printf(const char *format, ...)
{
	va_list ap;
	int n;

	va_start(ap, format);
	n = vprintf(format, ap);
	va_end(ap);

	return n;
}

