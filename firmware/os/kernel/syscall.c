/*
	System call handler functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, August 2015
*/

#include <kernel/syscall.h>


const syscall_table_entry_t g_syscalls[] =
{
    {1,     syscall_exit},
    {1,     syscall_console_putchar},
    {0,     syscall_console_getchar},
    {1,     syscall_leds},
    {0,     syscall_yield}
};


u32 syscall_exit(u32 code)
{
    UNUSED(code);
    /* TODO */

    return 0;
}


u32 syscall_console_putchar(u32 c)
{
    UNUSED(c);
    /* TODO */

    return 0;
}


u32 syscall_console_getchar()
{
    /* TODO */
    return 0;
}


u32 syscall_leds(u32 state)
{
    UNUSED(state);
    /* TODO */

    return 0;
}


u32 syscall_yield()
{
    /* TODO */
    return 0;
}
