#ifndef KERNEL_SYSCALL_H_INC
#define KERNEL_SYSCALL_H_INC
/*
	System call handler declarations

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, August 2015
*/

#include "include/defs.h"
#include "include/types.h"

#define MAX_SYSCALL		3			/* the highest-numbered system call, counting from zero */

/*
	System call numbers.  Numbers must start from zero and there must not be any gaps!
*/
#define SYS_exit    	        0
#define SYS_console_putchar     1
#define SYS_console_getchar	    2
#define SYS_leds                3

u32 syscall_exit();
u32 syscall_console_putchar();
u32 syscall_console_getchar();
u32 syscall_leds();

#endif
