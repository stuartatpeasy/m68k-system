/*
    Hashing functions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, November 2015.
*/

#include <kernel/util/kutil.h>


/*
	fnv1a32() - 32-bit Fowler-Noll-Vo hashing algorithm implementation
*/
u32 fnv1a32(const void *buf, u32 len)
{
	/* Fowler-Noll-Vo 32-bit initialisation constants */
	ku32 prime = 0x1000193;			/* 16777619 */
	ku32 offset = 0x811C9DC5;		/* 2166136261 */

	u8 *buf_ = (u8 *) buf;
	u32 hash;

	for(hash = offset; len; --len)
	{
		hash ^= *buf_++;
		hash *= prime;
	}

	return hash;
}

