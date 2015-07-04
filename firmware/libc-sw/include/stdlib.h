#ifndef __LIBCSW_STDLIB_H__
#define __LIBCSW_STDLIB_H__
/*
	stdlib.h - general-purpose standard library definitions

	part of libc-sw

	(c) Stuart Wallace <stuartw@atom.net>, 2011-08-14.
*/

#include "stddef.h"

/*
	FIXME: once a call structure has been established between apps and OS, this #include
	should no longer be necessary; instead, libc functions which require access to the
	underlying os_*() functions should make the appropriate call (e.g. a TRAP)
*/
#include "memory/kmalloc.h"

/*
	Type conversion
*/

double atof(const char *str);
int atoi(const char *str);
long atol(const char *str);
double strtod(const char *str, char **endptr);
long strtol(const char *str, char **end, int base);
unsigned long strtoul(const char *str, char **end, int base);
long long strtoll(const char *str, char **end, int base);
unsigned long long strtoull(const char *str, char **end, int base);


/*
	Pseudo-random sequence generation
*/

int rand(void);
int random(void);
void srand(unsigned int seed);
void srandom(unsigned int seed);


/*
	Memory allocation and deallocation
*/

void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void free(const void *ptr);

#endif

