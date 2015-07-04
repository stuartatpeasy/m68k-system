/*
	putchar.c - implementation of putchar()

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-10
*/

#include "stdio.h"


int putchar(int c)
{
	duarta_putc(c);

	return c;
}

