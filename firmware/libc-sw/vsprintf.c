/*
	vsprintf.c - implementation of vsprintf(), part of stdio

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-08
*/

#include "stdio.h"
#include "limits.h"


int vsprintf(char *str, const char *format, va_list ap)
{
	return vsnprintf(str, INT_MAX, format, ap);		/* Assume an unlimited buffer */
}

