/*
	Buddy memory allocator (second attempt)

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, July 2012.
 */

#include "buddy.h"

#include <stdio.h>		// FIXME: remove

void buddy_init(buddy_ctx * const ctx, void * const mem, u32 mem_len, u32 min_alloc_unit, 
					void * const tree)
{
	u32 i;

	ctx->tree = tree;
	ctx->mem = mem;

	/* ctx->min_alloc_unit = log2(min_alloc_unit) */
	for(ctx->min_alloc_unit = 0, --min_alloc_unit; min_alloc_unit;
			min_alloc_unit >>= 1, ctx->min_alloc_unit++) ;

	/* ctx->size = log2(size).  NOTE: will round up non-power-of-two blocks!! */
	for(ctx->size = 0, --mem_len; mem_len; mem_len >>= 1, ctx->size++) ;

	ctx->tree_end = 1 << (ctx->size - ctx->min_alloc_unit);

	for(i = 0; i < ctx->tree_end; ctx->tree[i++] = bb_null) ;
}


void buddy_dump(const buddy_ctx * const ctx)
{
	const int levels = ctx->size - ctx->min_alloc_unit + 1;
	int x, e;

	for(x = 0, e = 0; x < ctx->tree_end; ++e)
	{
		int sp, y;
		for(sp = 0; sp < (1 << (levels - e - 1)); ++sp)
			printf(" ");

		for(y = 0; y < (1 << e); y++)
		{
			int sp;
			switch(ctx->tree[x + y])
			{
				case bb_null: printf("-"); break;
				case bb_split: printf("S"); break;
				case bb_allocated: printf("A"); break;
				default: printf("?"); break;
			}

			for(sp = 0; sp < (1 << levels - e) - 1; ++sp)
				printf(" ");
		}
		x += y;
		puts("\n");
	}
}


void *buddy_malloc(buddy_ctx * const ctx, u32 size)
{
	s32 i, order, node_order;

	if(!size)
		return NULL;		/* cannot allocate 0 bytes */
	
	/* find the smallest 'order' such that 2^order >= size) */
	for(order = ctx->min_alloc_unit, --size, size >>= ctx->min_alloc_unit; size; size >>= 1)
		++order;

	node_order = ctx->size;		/* order of the root node == order of the whole region */
	i = 0;

printf("Region size = %d bytes (2^%d)\nRequested block order = %d\n",
		1 << ctx->size, ctx->size, order);

	/* find the leftmost free node */
	for(i = 0, node_order = ctx->size;
			(ctx->tree[i] != bb_null) && (LEFT_CHILD_INDEX(i) < ctx->tree_end);
			i = LEFT_CHILD_INDEX(i), --node_order) ;

printf("The leftmost node is %d (order %d)\n", i, node_order);

	while(node_order > order)
	{
		if(ctx->tree[i] == bb_null)
		{
			ctx->tree[i] = bb_split;
			i = LEFT_CHILD_INDEX(i);
			--node_order;
		}
	}

	if((node_order == order) && (ctx->tree[i] == bb_null))
	{
		ctx->tree[i] = bb_allocated;

		printf("Search complete. node = %d\n", i);

		if(i)
		{
			i -= (1 << (ctx->size - node_order)) - 1;
			printf("Node is at offset %d from the left edge of its level\n", i);

			return ctx->mem + (i << order);
		}
		else
			return ctx->mem;		/* allocated entire region */
	}

	printf("Failed to find free block (I think)\n");
	return NULL;

//	/* find the smallest free block that satisfies the request */
//
//	/* next in-order node */
//	{
//		if((RIGHT_CHILD_INDEX(i) < ctx->tree_end) && (ctx->tree[RIGHT_CHILD_INDEX(i)] != bb_null))
//		{
//			/* a right subtree exists.  next node is the leftmost node in the right subtree */
//			for(i = RIGHT_CHILD_INDEX(i);
//					(LEFT_CHILD_INDEX(i) < ctx->tree_end) 
//						&& (ctx->tree[LEFT_CHILD_INDEX(i)] != bb_null);
//					i = LEFT_CHILD_INDEX(i)) ;
//			
//		}
//		else
//		{
//			/* no right subtree exists.  next node is the first node upwards that does not
//			   contains this node in its right subtree */
//			for(; i && (i == RIGHT_SIBLING_INDEX(i)); i = PARENT_INDEX(i)) ;
//
//			i = i ? PARENT_INDEX(i) : -1;
//		}
//	}
//	
//	printf("The next in-order node is %d\n", i);

}


void buddy_free(buddy_ctx * const ctx, void *ptr)
{

}


const u32 buddy_get_free_space(const buddy_ctx * const ctx)
{

}


const u32 buddy_get_used_space(const buddy_ctx * const ctx)
{

}

