/*
    strings.c - implementation of various libc string-related functions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, 2012-2015.
*/

#include "strings.h"


/*
    bcopy()
*/
void bcopy(const void *src, void *dest, u32 n)
{
	const char *src_ = src;
	char *dest_ = dest;

	if(dest > src)
	{
		/* copy backwards */
		while(n--)
			dest_[n] = src_[n];
	}
	else if(dest < src)
	{
		/* copy forwards */
		while(n--)
			*dest_++ = *src_++;
	}
}


/*
    bzero()
*/
void bzero(void *s, u32 n)
{
	s8 *s_ = s;
	u32 nwords;

	/* Zero individual bytes until *s is word-aligned */
	for(; n && ((u32) s_ & 3); n--)
		*s_++ = 0;

	/* Zero words until n < 4 */
	for(nwords = n >> 2; nwords--; s_ += 4)
		*((u32 * const) s_) = 0;

	/* Zero trailing bytes */
	for(n &= 3; n--;)
        *s_++ = 0;
}
