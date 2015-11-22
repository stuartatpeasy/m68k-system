/*
	System call handler functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, August 2015
*/

#include <kernel/platform.h>      /* for plat_console_putc() */
#include <kernel/process.h>
#include <kernel/syscall.h>
#include <klibc/stdio.h>


/*
    System call table

    This must be kept in sync with the constants declared in <kernel/syscalls.h>.
*/
const syscall_table_entry_t g_syscalls[MAX_SYSCALL + 1] =
{
    {1,     syscall_exit},              /* Implemented in arch-specific asm */
    {1,     syscall_console_putchar},
    {0,     syscall_console_getchar},
    {1,     syscall_leds},
    {0,     syscall_yield}
};


/*
    syscall_console_putchar() - write a character to the console (blocking).
*/
s32 syscall_console_putchar(u32 c)
{
    return plat_console_putc(c);
}


/*
    syscall_console_getchar() - get a character from the console (blocking).
*/
s32 syscall_console_getchar()
{
    return plat_console_getc();
}


/*
    syscall_leds() - switch on/off the motherboard LEDs.
*/
s32 syscall_leds(u32 state)
{
    /* TODO */
    printf("syscall: leds(%d)\n", state);

    return 0;
}
