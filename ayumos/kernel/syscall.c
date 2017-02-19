/*
    System call handler functions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, August 2015
*/

#include <kernel/include/console.h>
#include <kernel/include/fs/file.h>
#include <kernel/include/process.h>
#include <kernel/include/syscall.h>
#include <klibc/include/stdio.h>


/*
    System call table

    This must be kept in sync with the constants declared in <kernel/syscalls.h>.
*/
const syscall_table_entry_t g_syscalls[MAX_SYSCALL + 1] =
{
    {0,     syscall_invalid},
    {1,     syscall_exit},              /* Implemented in arch-specific asm */
    {1,     syscall_console_putchar},
    {0,     syscall_console_getchar},
    {1,     syscall_leds},
    {0,     syscall_yield},
    {2,     syscall_open},
    {2,     syscall_create},
    {2,     syscall_close},
    {3,     syscall_read},
    {3,     syscall_write},
};


/*
    syscall_invalid() - handles all calls specifying an invalid syscall number.
*/
s32 syscall_invalid()
{
    return -ENOSYS;
}


/*
    syscall_console_putchar() - write a character to the console (blocking).
*/
s32 syscall_console_putchar(u32 c)
{
    return console_putc(c);
}


/*
    syscall_console_getchar() - get a character from the console (blocking).
*/
s32 syscall_console_getchar()
{
    return console_getc();
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
