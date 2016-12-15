#ifndef KERNEL_INCLUDE_SYSCALL_H_INC
#define KERNEL_INCLUDE_SYSCALL_H_INC
/*
	System call handler declarations

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, August 2015
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/include/syscalls.h>


typedef struct syscall_table_entry
{
    u32     num_args;
    void *  fn;         /* must be void* because number and type of args is unknown */
} syscall_table_entry_t;

/* System call table: maps number of arguments to syscall fn */
const syscall_table_entry_t g_syscalls[MAX_SYSCALL + 1];


s32 syscall_invalid();
s32 syscall_console_putchar(u32 c);
s32 syscall_console_getchar();
s32 syscall_leds(u32 state);

extern void syscall_yield();
extern void syscall_exit();

#endif
