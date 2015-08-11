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
    u32 x;

    /* Trim leading whitespace */
    for(; *src && isspace(*src); ++src)
        ;

    /* Find end of source string */
    for(x = strlen(src); x && (!src[x] || isspace(src[x])); --x)
        ;

    strncpy(dest, src, x);
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
