/*
    string.c: various string-manipulation functions not provided by klibc

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, July 2015.
*/

#include "kutil.h"
#include <ctype.h>
#include <string.h>


/*
    str_trim() - copy src into dest, trimming leading and trailing whitespace
*/
s8 *str_trim(s8 *dest, ks8 *src)
{
    s32 x;

    /* Trim leading whitespace */
    for(; isspace(*src); ++src)
        ;

    /* Find end of source string */
    for(x = strlen(src); x && isspace(src[x - 1]); --x)
        ;

    strncpy(dest, src, x);
    dest[x] = '\0';

    return dest;
}

/*
    strn_trim() - like str_trim(), but copies at most n chars into dest
    NOTE: src must be null-terminated
*/
s8 *strn_trim(s8 *dest, ks8 *src, u32 n)
{
    s32 x;

    /* Trim leading whitespace */
    for(; isspace(*src); ++src)
        ;

    /* Find end of source string */
    for(x = strlen(src); x && isspace(src[x - 1]); --x)
        ;

    if(x > n)
        x = n;

    strncpy(dest, src, x);

    if(x < n)
        dest[x] = '\0';

    return dest;
}



/*
    kstrerror() - extends libc-style strerror() to add descriptions of kernel-specific error codes
*/
ks8 *kstrerror(ks32 errnum)
{
    if(errnum < KERNEL_MIN_ERRNO)
        return strerror(errnum);

    switch(errnum)
    {
        case EUNKNOWN:          return "Unknown error";
        case EMEDIA:            return "Media error";
        case EMEDIACHANGED:     return "Media changed";
        case EDATA:             return "Data error";
        case ECKSUM:            return "Checksum error";
        case EBADSBLK:          return "Bad superblock";

        default:                return "Unrecognised error code";
    }
}


/*
    strn_trim_cpy()- copy a string from src, a buffer of length len, into dest.  Trim whitespace
    from both ends of src (which might not be zero-terminated) and zero-terminate the copy in dest.
    Note: dest must point to at least len+1 bytes of memory.
*/
s8 *strn_trim_cpy(s8 *dest, s8 *src, ku32 len)
{
    s8 *buffer;

    buffer = (s8 *) kmalloc(len + 1);
    if(!buffer)
        return NULL;    /* out of memory */

    buffer[len] = '\0';

    strncpy(buffer, src, len);
    str_trim(dest, buffer);
    kfree(buffer);

    return dest;
}
