/*
    stdio.c - implementation of various libc IO-related functions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, 2012-2015.
*/

#include <klibc/stdio.h>
#include <kernel/console.h>


/*
    putchar()
*/
s32 putchar(s32 c)
{
	while(console_putc(c) == -EAGAIN)
        ;

	return c;
}


/*
    put() - NOT LIBC-STANDARD
*/
s32 put(ks8 *s)
{
    for(; *s; ++s)
        while(console_putc(*s) == -EAGAIN)
            ;

	return SUCCESS;
}


/*
    puts()
*/
s32 puts(ks8 *s)
{
    for(; *s; ++s)
        while(console_putc(*s) == -EAGAIN)
            ;

    while(console_putc('\n') == -EAGAIN)
        ;

	return SUCCESS;
}
