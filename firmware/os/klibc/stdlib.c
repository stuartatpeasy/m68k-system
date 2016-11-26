/*
    stdlib.c - implementation of various libc functions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, 2012-2015.
*/

#include <klibc/stdlib.h>
#include <klibc/errno.h>


static s32 g_rand_next = 1;


/*
    calloc()
*/
void *calloc(ku32 nmemb, ku32 size)
{
    return kcalloc(nmemb, size);
}


/*
    free()
*/
void free(void *ptr)
{
    kfree(ptr);
}


/*
    malloc() - allocate *kernel* memory
*/
void *malloc(u32 size)
{
    return kmalloc(size);
}


/*
    rand()
*/
s32 rand()
{
    g_rand_next = g_rand_next * RAND_LCG_MULTIPLIER + RAND_LCG_INCREMENT;

	/* No scaling needed here as long as RAND_MAX = S32_MAX */
	return g_rand_next;
}


/*
    rand32() [non-standard]
*/
s32 rand32()
{
    g_rand_next = g_rand_next * RAND_LCG_MULTIPLIER + RAND_LCG_INCREMENT;
    return g_rand_next;
}


/*
    realloc()
*/
void *realloc(void *ptr, u32 size)
{
    return krealloc(ptr, size);
}


/*
    srand()
*/
void srand(ks32 seed)
{
    g_rand_next = seed;
}


/*
    strtoul()
*/
u32 strtoul(ks8 *nptr, s8 **endptr, s32 base)
{
	u32 n = 0;
	u64 n_;
	u8 digit = 0;
	s8 digit_found = 0, overflow = 0, neg = 0;
	ks8 * const nptr_ = nptr;

	/* skip over whitespace */
	while((*nptr == ' ') || (*nptr == '\t') || (*nptr == '\r') || (*nptr == '\n'))
		++nptr;

	/* any -ve number equates to zero */
	if(*nptr == '-')
	{
		neg = 1;
		++nptr;
	}

	/* read optional '+' character */
	else if(*nptr == '+')
		++nptr;

	/* check that base is valid, i.e. it lies between 2 and 36. */
	if(base && ((base < 2) || (base > 36)))
	{
	    errno = EINVAL;
		return 0;
	}

	/*
		if base is zero:
			- look for a "0x" prefix: if present, assume base 16
			- look for a "0" prefix: if present, assume base 8
			- else assume base 10.
	*/
	if(!base)
	{
		if(*nptr == '0')
		{
			if((*(nptr + 1) == 'x') || (*(nptr + 1) == 'X'))
				base = 16;
			else
				base = 8;
		}
		else
			base = 10;
	}

	/* if base == 16 and there is a "0x" or "0X" prefix, step over the prefix. */
	if((base == 16) && (*nptr == '0') && ((*(nptr + 1) == 'x') || (*(nptr + 1) == 'X')))
		nptr += 2;

	/* process numeric characters */
	for(; *nptr; ++nptr)
	{
		if((*nptr >= '0') && (*nptr <= '9'))
			digit = *nptr - '0';
		else if((*nptr >= 'a') && (*nptr <= 'z'))
			digit = (*nptr - 'a') + 10;
		else if((*nptr >= 'A') && (*nptr <= 'Z'))
			digit = (*nptr - 'A') + 10;
		else
			digit = 255;		/* flag this digit as invalid */

		if(digit >= base)
		{
			/* invalid digit */
			if(endptr)
			{
				if(digit_found)
					*endptr = (s8 *) nptr;
				else
					*endptr = (s8 *) nptr_;
			}
			return n;
		}
		else
			digit_found = 1;

		/* tedious way of avoiding 32-bit multiplies (expensive on a 16-bit platform).  Also detects over/underflow */
		n_ = 0;
		if(base & 0x20) n_ += n << 5;
		if(base & 0x10) n_ += n << 4;
		if(base & 0x8)  n_ += n << 3;
		if(base & 0x4)  n_ += n << 2;
		if(base & 0x2)  n_ += n << 1;
		if(base & 0x1)  n_ += n;

		if(n_ > 0xffffffff)
			overflow = 1;

		n = (u32) n_ + digit;;
	}

	if(endptr)
		*endptr = (s8 *) nptr;

	if(overflow)
	{
	    errno = ERANGE;
		return U32_MAX;;
	}

	return neg ? -n : n;
}
