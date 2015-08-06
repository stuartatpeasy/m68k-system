#ifndef INCLUDE_SYSCALL_H_INC
#define INCLUDE_SYSCALL_H_INC
/*
	System call declarations

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, May 2012.
*/

#define MAX_SYSCALL		3			/* the highest-numbered system call, counting from zero */

/*
	System call numbers.  Numbers must start from zero and there must not be any gaps!
*/
#define SYSCALL_null	0
#define SYSCALL_blah	1
#define SYSCALL_meh		2
#define SYSCALL_foo		3

#endif

