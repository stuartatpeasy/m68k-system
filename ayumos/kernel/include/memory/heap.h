#ifndef KERNEL_INCLUDE_MEMORY_HEAP_H_INC
#define KERNEL_INCLUDE_MEMORY_HEAP_H_INC
/*
	heap.h: declarations of functions and types for the simple and rubbish heap memory allocator

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, 7th August 2011.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>

#ifdef KMALLOC_HEAP

typedef struct heap_
{
	u8 *			start;
	unsigned int	size;
} heap_ctx;


void heap_init(heap_ctx * const heap, void * const mem, u32 mem_len);
void *heap_malloc(heap_ctx * const heap, u32 size);
void *heap_calloc(heap_ctx * const heap, ku32 nmemb, ku32 size);
void *heap_realloc(heap_ctx * const heap, const void *ptr, u32 size);
void heap_free(heap_ctx * const heap, const void *ptr);
u32 heap_freemem(heap_ctx * const heap);
u32 heap_usedmem(heap_ctx * const heap);

#endif	/* KMALLOC_HEAP */

#endif

