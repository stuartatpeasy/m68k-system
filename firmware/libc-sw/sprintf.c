/*
	sprintf.c - implementation of sprintf(), part of stdio

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-08
*/

#include "stdio.h"
#include "limits.h"


int sprintf(char *str, const char *format, ...)
{
	va_list ap;
	int n;

	va_start(ap, format);
	n = vsnprintf(str, INT_MAX, format, ap);
	va_end(ap);

	return n;
}

