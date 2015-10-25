/*
	System call handler functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, August 2015
*/

#include <kernel/sched.h>
#include <kernel/syscall.h>
#include <klibc/stdio.h>
#include <platform/platform.h>      /* for plat_console_putc() */


/*
    System call table

    This must be kept in sync with the constants declared in <kernel/syscalls.h>.
*/
const syscall_table_entry_t g_syscalls[] =
{
    {1,     syscall_exit},
    {1,     syscall_console_putchar},
    {0,     syscall_console_getchar},
    {1,     syscall_leds},
    {0,     syscall_yield}
};


/*
    syscall_exit() - terminate the current process.
*/
s32 syscall_exit(u32 code)
{
    /* TODO */
    cpu_disable_interrupts();

    printf("Process %d exited with code %d\n", sched_get_pid(), code);

    while(1)
        ;       /* FIXME - remove process from scheduler list; get on with something else... */

    return 0;
}


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
    /* TODO */
    puts("syscall: console_getchar()");

    return 0;
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


/*
    syscall_yield() - yield the remainder of the current process's time-slice.
*/
s32 syscall_yield()
{
    /* TODO */
    puts("syscall: yield()");

    return 0;
}
