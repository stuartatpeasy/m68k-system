#ifndef KLIBC_STDLIB_H_INC
#define KLIBC_STDLIB_H_INC
/*
    stdlib.c - declaration of various libc functions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, 2012-2015.
*/

#include "../include/limits.h"
#include "../include/types.h"
#include "../memory/kmalloc.h"


void *calloc(ku32 nmemb, ku32 size);
void free(void *ptr);
void *malloc(u32 size);
void *realloc(void *ptr, u32 size);
u32 strtoul(ks8 *nptr, s8 **endptr, s32 base);

#endif
