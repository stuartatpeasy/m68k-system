/*
    stdio.c - implementation of various libc IO-related functions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, 2012-2015.
*/

#include "stdio.h"


/*
    putchar()
*/
s32 putchar(s32 c)
{
	duarta_putc(c);

	return c;
}


/*
    put() - NOT LIBC-STANDARD
*/
s32 put(ks8 *s)
{
	while(*s)
		duarta_putc(*s++);

	return SUCCESS;
}


/*
    puts()
*/
s32 puts(ks8 *s)
{
	while(*s)
		duarta_putc(*s++);
	duarta_putc('\n');

	return SUCCESS;
}
