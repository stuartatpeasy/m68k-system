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
    {1,     syscall_leds}
};


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
