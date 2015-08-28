#ifndef INCLUDE_SYSCALL_H_INC
#define INCLUDE_SYSCALL_H_INC
/*
	System call declarations

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, May 2012.
*/

#define MAX_SYSCALL		3			/* the highest-numbered system call, counting from zero */

/* System call with one argument */
#define SYSCALL1(func, arg)                 \
    asm volatile                            \
    (                                       \
        "movel %0, %%d0       \n"           \
        "movel %1, %%d1       \n"           \
        "trap #1              \n"           \
        : /* no output operands */          \
        : "r" (func), "m" (arg)             \
        : "cc", "d0", "d1"                  \
    );


/*
	System call numbers.  Numbers must start from zero and there must not be any gaps!
*/
#define SYS_exit    	        0
#define SYS_console_putchar     1
#define SYS_console_getchar	    2
#define SYS_leds                3

#endif

