#ifndef KLIBC_STRING_H_INC
#define KLIBC_STRING_H_INC

/*
    string.h - declaration of various libc string-related functions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, 2012-2015.
*/

#include "include/types.h"
#include "include/limits.h"
#include "stdlib.h"
#include "errno.h"

s32 memcmp(const void *s1, const void *s2, u32 n);
void *memcpy(void *dest, const void *src, u32 n);
void *memset(void *src, s32 c, s32 n);
s8 *strcat(s8 *dest, ks8 *src);
s32 strcmp(ks8 *s1, ks8 *s2);
s8 *strcpy(s8 *dest, ks8 *src);
s8 *strdup(ks8 *s);
ks8 *strerror(ks32 errnum);
u32 strlen(const s8 *s);
s32 strncmp(ks8 *s1, ks8 *s2, u32 n);
s8 *strncpy(s8 *dest, ks8 *src, u32 n);
s8 *strstr(ks8 *haystack, ks8 *needle);

#endif
