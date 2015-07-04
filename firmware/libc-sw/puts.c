/*
	puts.c - implementation of puts(), part of stdio

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-08
*/

#include "stdio.h"


int puts(const char *s)
{
	while(*s)
		duarta_putc(*s++);
	duarta_putc('\r');

	return 0;
}

