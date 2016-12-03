/*
	kmalloc.c - kernel memory allocation/deallocation functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, 2012-07
*/

#include <kernel/include/memory/kmalloc.h>


/* mem_ctx is a struct defined by whichever allocator is selected */
mem_ctx g_kheap;    /* kernel heap */
mem_ctx g_uheap;    /* user heap (shared by all userland processes) */

#if defined(KMALLOC_HEAP)

void kmeminit(void * const start, void * const end)
{
	heap_init(&g_kheap, start, end - start);
}

void umeminit(void * const start, void * const end)
{
    heap_init(&g_uheap, start, end - start);
}

#elif defined(KMALLOC_BUDDY)

/* This allocator really isn't going to work without a lot more work. */

/* The buddy allocator uses a "map" to store the alloc/free state of blocks.  The size of this
 * map is 2^(log2(pool_size) - log2(smallest_allocatable_block)) bytes.  With the current default
 * sizes (pool_size=128K, smallest_allocatable_block=0.5K) the map occupies 256 bytes. */

s8 g_buddy_map[1 << (OS_HEAP_SIZE_LOG2 - BUDDY_MIN_ALLOC_UNIT)];

void kmeminit(void * const start, void * const end)
{
	buddy_init(&g_kheap, start, end - start,
					BUDDY_MIN_ALLOC_UNIT, g_buddy_map);
}

void umeminit(void * const start, void * const end)
{
    buddy_init(&g_uheap, start, end - start,
                    BUDDY_MIN_ALLOC_UNIT, g_buddy_map);
}

#endif

/*
    Allocation functions for kernel memory space
*/

void *kmalloc(u32 size)
{
	return ALLOCATOR_FN(malloc)(&g_kheap, size);
}

void *kcalloc(ku32 nmemb, ku32 size)
{
	return ALLOCATOR_FN(calloc)(&g_kheap, nmemb, size);
}

void *krealloc(void *ptr, u32 size)
{
	return ALLOCATOR_FN(realloc)(&g_kheap, ptr, size);
}

void kfree(void *ptr)
{
	ALLOCATOR_FN(free)(&g_kheap, ptr);
}

u32 kfreemem()
{
	return ALLOCATOR_FN(freemem)(&g_kheap);
}

u32 kusedmem()
{
	return ALLOCATOR_FN(usedmem)(&g_kheap);
}


/*
    Allocation functions for user memory space
*/

void *umalloc(u32 size)
{
	return ALLOCATOR_FN(malloc)(&g_uheap, size);
}

void *ucalloc(ku32 nmemb, ku32 size)
{
	return ALLOCATOR_FN(calloc)(&g_uheap, nmemb, size);
}

void *urealloc(void *ptr, u32 size)
{
	return ALLOCATOR_FN(realloc)(&g_uheap, ptr, size);
}

void ufree(void *ptr)
{
	ALLOCATOR_FN(free)(&g_uheap, ptr);
}

u32 ufreemem()
{
	return ALLOCATOR_FN(freemem)(&g_uheap);
}

u32 uusedmem()
{
	return ALLOCATOR_FN(usedmem)(&g_uheap);
}
