/*
	kmalloc.c - kernel memory allocation/deallocation functions

	Part of the as-yet-unnamed MC68010 operating system

	(c) Stuart Wallace <stuartw@atom.net>, 2012-07

	FIXME - panic on allocation failure
*/

#include "memory/kmalloc.h"

/* heap_ctx is a struct defined by whichever allocator is selected */
heap_ctx g_heap;

#if defined(KMALLOC_HEAP)

void kmeminit()
{
	heap_init(&g_heap, (void *) OS_HEAP_START, OS_HEAP_END - OS_HEAP_START);
}

#elif defined(KMALLOC_BUDDY)

/* The buddy allocator uses a "map" to store the alloc/free state of blocks.  The size of this
 * map is 2^(log2(pool_size) - log2(smallest_allocatable_block)) bytes.  With the current default
 * sizes (pool_size=128K, smallest_allocatable_block=0.5K) the map occupies 256 bytes. */

s8 g_buddy_map[1 << (OS_HEAP_SIZE_LOG2 - BUDDY_MIN_ALLOC_UNIT)];

void kmeminit()
{
	buddy_init(&g_heap, (void *) OS_HEAP_START, OS_HEAP_END - OS_HEAP_START,
					BUDDY_MIN_ALLOC_UNIT, g_buddy_map);
}

#endif


void *kmalloc(u32 size)
{
	return ALLOCATOR_FN(malloc)(&g_heap, size);
}


void *kcalloc(ku32 nmemb, ku32 size)
{
	return ALLOCATOR_FN(calloc)(&g_heap, nmemb, size);
}


void *krealloc(void *ptr, u32 size)
{
	return ALLOCATOR_FN(realloc)(&g_heap, ptr, size);
}


void kfree(void *ptr)
{
	ALLOCATOR_FN(free)(&g_heap, ptr);
}


u32 kfreemem()
{
	return ALLOCATOR_FN(freemem)(&g_heap);
}


u32 kusedmem()
{
	return ALLOCATOR_FN(usedmem)(&g_heap);
}

