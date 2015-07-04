/*
	bzero.c

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-08-29


	This function should be optimised extensively.  It would be worth coding it in assembly
	language, as it is quite performance-critical.
*/

#include "strings.h"


void bzero(void *s, size_t n)
{
	char *s_ = s;

	/* Zero individual bytes until *s is word-aligned */
	for(; n && ((unsigned int) s_ & 3); n--)
		*s_++ = 0;

	/* Zero words until n < 4 */
	for(; n & 3; n -= 4, s_ += 4)
		*((unsigned int * const) s_) = 0;

	/* Zero trailing bytes */
	for(; n; n--, *s_++ = 0) ;
}

