#ifndef KERNEL_MEMORY_BUDDY_H_INC
#define KERNEL_MEMORY_BUDDY_H_INC
/*
	buddy.h: declarations for buddy memory allocator

	Part of the as-yet-unnamed MC68010 operating system.


	(c) Stuart Wallace, May 2012.
*/

#include <stdio.h>
#include <include/types.h>
#include <kernel/util/kutil.h>

#ifdef KMALLOC_BUDDY

#define BUDDY_ABS(x)	({ __typeof__ (x) _x = (x); ((_x >= 0) ? (_x) : -(_x)); })


typedef struct
{
	s8 *map;			/* vector containing block sizes and free/allocated status	*/
	u32 end;			/* current length of map									*/
	void *mem;			/* pointer to the start of the allocatable region			*/
	u32 size;			/* size of the block as a power of two						*/
	u32 min_alloc_unit;	/* minimum allocation unit as a power of two				*/
} buddy_ctx;


void buddy_init(buddy_ctx * const ctx, void * const mem, u32 mem_len, u32 min_alloc_unit, s8 *map);
void buddy_dump(const buddy_ctx * const ctx);
void *buddy_malloc(buddy_ctx * const ctx, u32 size);
void buddy_free(buddy_ctx * const ctx, void *ptr);
u32 buddy_get_free_space(const buddy_ctx * const ctx);
u32 buddy_get_used_space(const buddy_ctx * const ctx);

#endif /* KMALLOC_BUDDY */

#endif

