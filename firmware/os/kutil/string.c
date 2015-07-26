/*
    string.c: various string-manipulation functions not provided by klibc

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, July 2015.
*/

#include "kutil.h"
#include <ctype.h>
#include <string.h>


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
