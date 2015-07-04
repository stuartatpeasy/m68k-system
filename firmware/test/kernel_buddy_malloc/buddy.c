/*
	buddy.c: buddy memory allocator

	Part of the as-yet-unnamed MC68010 operating system.

	(c) Stuart Wallace, May 2012.

*/

#include "buddy.h"
#include <stdio.h>		// FIXME: remove

#define BUDDY_DOUBLE_COALESCE	1


void buddy_init(buddy_ctx * const ctx, void * const mem, u32 mem_len, u32 min_alloc_unit, s8 *map)
{
	ctx->map = map;
	ctx->mem = mem;

	for(ctx->min_alloc_unit = 0, --min_alloc_unit; min_alloc_unit; 
			min_alloc_unit >>= 1, ctx->min_alloc_unit++) ;

	for(ctx->size = 0, --mem_len; mem_len; mem_len >>= 1, ctx->size++) ;

	ctx->map[0] = ctx->size;
	ctx->end = 1;
}


void buddy_dump(const buddy_ctx * const ctx)
{
	int i;
	for(i = 0; i < ctx->end; i++)
		printf("%3d ", ctx->map[i]);
	puts("");
}


void *buddy_malloc(buddy_ctx * const ctx, u32 size)
{
	unsigned int min_index = -1, min_order = -1, offset = 0, pool_pos = 0, order, i;

	if(!size)
		return NULL;		/* cannot allocate 0 bytes */

	/* find the smallest 'order' such that 2^order >= size */
	for(order = ctx->min_alloc_unit, --size, size >>= ctx->min_alloc_unit; size; size >>= 1)
		++order;

	/* find smallest free block that satisfies request */
	for(i = 0; i < ctx->end; i++)
	{
		if((ctx->map[i] >= order) && (ctx->map[i] < min_order))
		{
			min_order = ctx->map[i];
			min_index = i;
			offset = pool_pos;
		}
		pool_pos += 1 << BUDDY_ABS(ctx->map[i]);
	}

	/* split block, if necessary, until it is as small as required */
	if(min_order > order)
	{
		int j;

		/* make space for the split blocks */
		for(j = ctx->end; --j > min_index; ctx->map[j + (min_order - order)] = ctx->map[j]) ;

		ctx->end += min_order - order;
		ctx->map[min_index] = order;

		/* write the components of the split blocks */
		for(j = min_index + 1; order < min_order; ctx->map[j++] = order++) ;
	}

	if(min_order == order)
	{
		/* found a suitable block; mark it allocated and return a ptr */
		ctx->map[min_index] = -ctx->map[min_index];
		return ctx->mem + offset;
	}

	return NULL;	/* no suitable block found */
}


void buddy_free(buddy_ctx * const ctx, void *ptr)
{
	unsigned int p_ = ptr - ctx->mem;	/* = offset of ptr from start of mem block */
	unsigned int x;
	int i;

	if(p_ >= (1 << ctx->size))
		return;						/* XXX error: invalid ptr */

	while(1)
	{
		int i, i_;

		/* locate block */
		for(i = x = 0; x < p_; x += 1 << BUDDY_ABS(ctx->map[i++])) ;

		if((x > p_) || (ctx->map[i] > 0))
			return;						/* XXX error: invalid ptr */

		ctx->map[i] = -ctx->map[i];		/* mark block as free */

		/* locate block's buddy */
		p_ ^= 1 << ctx->map[i];

		for(i_ = x = 0; x < p_; x += 1 << BUDDY_ABS(ctx->map[i_++])) ;

		if(ctx->map[i] > 0)
		{
			printf("*** buddy is free ***  Going to merge indices %d and %d\n", i, i_);
			if(i > i_)
				i = i_;

			ctx->map[i]++;
			for(i_ = i + 1; i_ < (ctx->end - 1); ++i_)
				ctx->map[i_] = ctx->map[i_ + 1];

			--ctx->end;
		}
		else
			break;
	}

	/* coalesce adjacent free blocks */
	while(1)
	{
		int j;
		if(i && (ctx->map[i - 1] == ctx->map[i]))
		{
			/* coalesce left */
			ctx->map[i - 1]++;
			for(j = i; j < (ctx->end - 1); ++j)
				ctx->map[j] = ctx->map[j + 1];

			--ctx->end;
			--i;
		}
		else if((i < (ctx->end - 1)) && (ctx->map[i] == ctx->map[i + 1]))
		{
			/* coalesce right */
			ctx->map[i]++;
			for(j = i + 1; j < (ctx->end - 1); ++j)
				ctx->map[j] = ctx->map[j + 1];

			--ctx->end;
		}
#ifdef BUDDY_DOUBLE_COALESCE
		else if((i > 1) && (ctx->map[i] == ctx->map[i - 1] - 1) 
				&& (ctx->map[i] == ctx->map[i - 2]))
		{
			/* double-coalesce left */
			ctx->map[i - 2] += 2;
			for(j = i; j < (ctx->end - 2); ++j)
				ctx->map[j] = ctx->map[j + 2];

			ctx->end -= 2;
			i -= 2;
		}
		else if((i < (ctx->end - 2)) && (ctx->map[i] == ctx->map[i + 1] - 1)
				&& (ctx->map[i] == ctx->map[i + 2]))
		{
			/* double-coalesce right */
			ctx->map[i] += 2;

			for(j = i + 1; j < (ctx->end - 2); ++j)
				ctx->map[j] = ctx->map[j + 2];

			ctx->end -= 2;
		}
#endif
		else break;
	}
}


u32 buddy_get_free_space(const buddy_ctx * const ctx)
{
	u32 i, free_bytes = 0;

	for(i = 0; i < ctx->end; i++)
		if(ctx->map[i] > 0)
			free_bytes += 1 << ctx->map[i];

	return free_bytes;
}


u32 buddy_get_used_space(const buddy_ctx * const ctx)
{
	u32 i, used_bytes = 0;

	for(i = 0; i < ctx->end; i++)
		if(ctx->map[i] < 0)
			used_bytes += 1 << -ctx->map[i];

	return used_bytes;
}

