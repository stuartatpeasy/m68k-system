/*
	put.c - implementation of put(), an extension to stdio

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-08
*/

#include "stdio.h"


int put(const char *s)
{
	while(*s)
		duarta_putc(*s++);

	return 0;
}

