#ifndef KLIBC_STDIO_H_INC
#define KLIBC_STDIO_H_INC

/*
    stdio.h - declaration of various libc IO-related functions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, 2012-2015.
*/

#include <stdarg.h>

#include "device/duart.h"
#include "include/limits.h"
#include "include/types.h"
#include "stdlib.h"

s32 printf(const char *format, ...);
s32 putchar(s32 c);
s32 put(ks8 *s);
s32 puts(ks8 *s);
s32 snprintf(char *str, u32 size, const char *format, ...);
s32 sprintf(char *str, const char *format, ...);
s32 vprintf(const char *format, va_list ap);
s32 vsnprintf(char *str, u32 size, const char *format, va_list ap);
s32 vsprintf(char *str, const char *format, va_list ap);

#endif
