#ifndef KLIBC_STDLIB_H_INC
#define KLIBC_STDLIB_H_INC
/*
    stdlib.c - declaration of various libc functions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, 2012-2015.
*/

#include <kernel/include/limits.h>
#include <kernel/include/types.h>
#include <kernel/include/memory/kmalloc.h>

#if !defined(HOST_HARNESS)
#define RAND_MAX (2147483647)
#endif

/* Parameters for the linear congruential generator used by rand(), srand(), rand32(), etc. */
#define RAND_LCG_MULTIPLIER     (1103515245)
#define RAND_LCG_INCREMENT      (12345)


void *calloc(ku32 nmemb, ku32 size);
void free(void *ptr);
void *malloc(u32 size);
char *path_canonicalise(char *path);
s32 rand();
s32 rand32();
void *realloc(void *ptr, u32 size);
void srand(ks32 seed);
u32 strtoul(ks8 *nptr, s8 **endptr, s32 base);

#endif
