#ifndef KERNEL_MEMORY_HEAP_H_INC
#define KERNEL_MEMORY_HEAP_H_INC
/*
	heap.h: declarations of functions and types for the simple and rubbish heap memory allocator

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, 7th August 2011.
*/

#include <include/types.h>

#ifdef KMALLOC_HEAP

typedef struct heap_
{
	void *			start;
	unsigned int	size;
} heap_ctx;


void heap_init(heap_ctx * const heap, void * const mem, u32 mem_len);
void *heap_malloc(const heap_ctx * const heap, u32 size);
void *heap_calloc(const heap_ctx * const heap, ku32 nmemb, ku32 size);
void *heap_realloc(const heap_ctx * const heap, const void *ptr, u32 size);
void heap_free(const heap_ctx * const heap, const void *ptr);
u32 heap_freemem(const heap_ctx * const heap);
u32 heap_usedmem(const heap_ctx * const heap);

#endif	/* KMALLOC_HEAP */

#endif

