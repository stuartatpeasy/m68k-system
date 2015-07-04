#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"

/*
	This file implements kmalloc()/kcalloc()/kfree() using the local system's allocator.
*/


void *kmalloc(u32 size)
{
	void *p = malloc(size);

#ifdef DEBUG_ALLOC
	printf("[kmalloc(%u) = 0x%p]\n", size, p);
#endif

	return p;
}


void *kcalloc(ku32 nmemb, ku32 size)
{
	void *p = calloc(nmemb, size);

#ifdef DEBUG_ALLOC
	printf("[kcalloc(%u, %u) (=%u bytes) = 0x%p]\n", nmemb, size, nmemb * size, p);
#endif

	return p;
}


void *krealloc(void *ptr, u32 size)
{
	void *p = realloc(ptr, size);

#ifdef DEBUG_ALLOC
	printf("[krealloc(0x%p, %u) = 0x%p]\n", ptr, size, p);
#endif

	return p;
}


void kfree(void *ptr)
{
#ifdef DEBUG_ALLOC
	printf("[kfree(0x%p)]\n", ptr);
#endif

	free(ptr);
}

void *kmemcpy(void *dest, const void *src, u32 n)
{
	return memcpy(dest, src, n);
}

