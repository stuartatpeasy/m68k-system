/*
	System call handler functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, August 2015
*/

#include "kernel/syscall.h"


u32 syscall_exit(u32 code)
{
    return 0;
}


u32 syscall_console_putchar(u32 c)
{
    return 0;
}


u32 syscall_console_getchar()
{
    return 0;
}


u32 syscall_leds(u32 state)
{
    return 0;
}
