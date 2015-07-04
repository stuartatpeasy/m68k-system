#ifndef __BUDDY_H__
#define __BUDDY_H__
/*
	Buddy memory allocator (second attempt)

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, July 2012.
 */

#include "include/types.h"

/* Compute the number of bytes required for the tree representing a buddy structure of size
   2^size with minimum allocation unit size 2^min_block_size */
#define BUDDY_TREE_LEN(size, min_block_size)	(1 << ((size) - (min_block_size)))

#define LEFT_CHILD_INDEX(i)		(((i) << 1) + 1)
#define RIGHT_CHILD_INDEX(i)	(((i) << 1) + 2)

#define PARENT_INDEX(i)			(((i) - 1) << 1)

#define IS_LEFT_NODE(i)			((i) & 1)
#define IS_RIGHT_NODE(i)		(!((i) & 1))

#define SIBLING(i)				((i) + IS_LEFT_NODE(i) ? 1 : -1)	/* NOTE: evaluates i twice */

#define LEFT_SIBLING_INDEX(i)	((i) & ~1)
#define RIGHT_SIBLING_INDEX(i)	((i) | 1)

/* enum to represent the three states that a buddy block tree node can be in */
typedef enum
{
	bb_null, bb_split, bb_allocated
} buddy_block;

/* buddy allocator context */
typedef struct
{
	void *mem;				/* ptr to the start of allocatable memory		*/
	buddy_block *tree;		/* tree containing allocation data				*/
	u8 tree_end;			/* number of elements in the tree				*/
	u8 size;				/* log2(size of allocatable block in bytes)		*/
	u8 min_alloc_unit;		/* log2(smallest allocatable block in bytes)	*/
} buddy_ctx;

void buddy_init(buddy_ctx * const ctx, void * const mem, u32 mem_len, u32 min_alloc_unit,
					void * const tree);
void buddy_dump(const buddy_ctx * const ctx);
void *buddy_malloc(buddy_ctx * const ctx, u32 size);
void buddy_free(buddy_ctx * const ctx, void *ptr);
const u32 buddy_get_free_space(const buddy_ctx * const ctx);
const u32 buddy_get_used_space(const buddy_ctx * const ctx);

#endif

