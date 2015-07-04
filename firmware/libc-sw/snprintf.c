/*
	snprintf.c - implementation of snprintf(), part of stdio

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-08
*/

#include "stdio.h"


int snprintf(char *str, size_t size, const char *format, ...)
{
	va_list ap;
	int n;

	va_start(ap, format);
	n = vsnprintf(str, size, format, ap);
	va_end(ap);

	return n;
}

