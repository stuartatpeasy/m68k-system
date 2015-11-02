#ifndef KERNEL_SYSCALLS_H_INC
#define KERNEL_SYSCALLS_H_INC
/*
	System call number declarations

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, October 2015

    NOTE: this file needs to be parsed by both the C compiler and the assembler.  Do not put any
    directives incomprehensible to either in here.
*/


/*
	System call numbers.  Numbers must start from zero and there must not be any gaps!
*/
#define SYS_exit    	        0       /* called by user process in order to terminate     */
#define SYS_console_putchar     1       /* write a character to the console                 */
#define SYS_console_getchar	    2       /* read a character from the console                */
#define SYS_leds                3       /* switch on/off the motherboard LEDs               */
#define SYS_yield               4       /* yield the remainder of the process quantum       */


/* The highest system call number */
#define MAX_SYSCALL             4

#endif