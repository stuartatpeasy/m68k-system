#ifndef KLIBC_STRINGS_H_INC
#define KLIBC_STRINGS_H_INC

/*
    strings.h - declaration of various libc string-related functions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, 2012-2015.
*/

#include "include/types.h"


void bcopy(const void *src, void *dest, u32 n);
void bzero(void *s, u32 n);


#endif
