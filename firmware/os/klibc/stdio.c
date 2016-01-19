/*
    stdio.c - implementation of various libc IO-related functions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, 2012-2015.
*/

#include <klibc/stdio.h>
#include <kernel/platform.h>


/*
    putchar()
*/
s32 putchar(s32 c)
{
	plat_console_putc(c);

	return c;
}


/*
    put() - NOT LIBC-STANDARD
*/
s32 put(ks8 *s)
{
	while(*s)
		plat_console_putc(*s++);

	return SUCCESS;
}


/*
    puts()
*/
s32 puts(ks8 *s)
{
	while(*s)
		plat_console_putc(*s++);
	plat_console_putc('\n');

	return SUCCESS;
}
