#ifndef __LIBCSW_STDIO_H__
#define __LIBCSW_STDIO_H__
/*
	stdio.h - declarations for standard input/output functions

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-08-14.
*/

#include <stdarg.h>
#include "stddef.h"
#include "duart.h"			/* FIXME: remove this */


int printf(const char *format, ...);
int put(const char *s);
int putchar(int c);
int puts(const char *s);
int snprintf(char *str, size_t size, const char *format, ...);
int sprintf(char *str, const char *format, ...);
int vprintf(const char *format, va_list ap);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);
int vsprintf(char *str, const char *format, va_list ap);


#endif

