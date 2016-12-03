/*
    strings.c - implementation of various libc string-related functions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, 2012-2015.
*/

#include <klibc/include/strings.h>


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
	for(; ((u32) s_ & 3) && n; n--)
		*s_++ = 0;

	/* Zero words until n < 4 */
	for(nwords = n >> 2; nwords--; s_ += 4)
		*((u32 * const) s_) = 0;

	/* Zero trailing bytes */
	for(n &= 3; n--;)
        *s_++ = 0;
}


/*
    strcasecmp() - case-insensitive strcmp()
*/
s32 strcasecmp(ks8 *s1, ks8 *s2)
{
	for(; tolower(*s1) == tolower(*s2); ++s1, ++s2)
		if(*s1 == 0)
			return 0;

	return tolower(*s1) - tolower(*s2);
}
