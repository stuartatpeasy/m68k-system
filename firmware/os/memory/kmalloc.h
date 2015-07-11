#ifndef __MEMORY_KMALLOC_H__
#define __MEMORY_KMALLOC_H__
/*
	kmalloc.h - kernel memory allocation/deallocation functions

	Part of the as-yet-unnamed MC68010 operating system

	(c) Stuart Wallace <stuartw@atom.net>, July 2012
*/

#include "include/types.h"
#include "memory/memorymap.h"

/* If no allocator was specified in build options, use the heap allocator */
#if (!defined(KMALLOC_HEAP) && !defined(KMALLOC_BUDDY))
#define KMALLOC_HEAP
#endif

#if defined(KMALLOC_HEAP)
/*
	Use heap allocator
*/

#include "memory/heap.h"

#define ALLOCATOR_FN(name) heap_##name
typedef heap_ctx mem_ctx;

#elif defined(KMALLOC_BUDDY)
/*
	Use buddy allocator
*/

#ifndef BUDDY_MIN_ALLOC_UNIT
#define BUDDY_MIN_ALLOC_UNIT	(512)		/* bytes */
#endif

#include "memory/buddy.h"

#define ALLOCATOR_FN(name) buddy_##name
typedef buddy_ctx mem_ctx;

#else
#error "No memory allocator specified (try -DKMALLOC_HEAP or -DKMALLOC_BUDDY)"
#endif

extern mem_ctx g_Heap;

void kmeminit(void * const start, void * const end);
void *kmalloc(u32 size);
void *kcalloc(ku32 nmemb, ku32 size);
void *krealloc(void *ptr, u32 size);
void kfree(void *ptr);
u32 kfreemem();
u32 kusedmem();

#endif
