#ifndef __STRING_H__
#define __STRING_H__
/*
	string.h - declarations for general-purpose string functions

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-07-*.
*/

#include "stddef.h"


void *memcpy(void *dest, const void *src, size_t n);
void *memset(const void *s, int c, size_t n);
size_t strlen(const char *);
int strcmp(const char *, const char *);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
char *strstr(const char *haystack, const char *needle);

#endif

