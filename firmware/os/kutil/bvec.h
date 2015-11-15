#ifndef KUTIL_BVEC_H_INC
#define KUTIL_BVEC_H_INC
/*
	bvec.h - block-allocating dynamic array (vector) implementation

	Part of the as-yet-unnamed MC68010 operating system

	(c) Stuart Wallace <stuartw@atom.net>, 2012-12

	See comments in bvec.c for a description of this module.
*/

#include <include/defs.h>
#include <include/types.h>
#include <kernel/memory/kmalloc.h>
#include <klibc/errno.h>
#include <klibc/string.h>
#include <kutil/kutil.h>


/* pointers to blocks will be allocated in units of BVEC_BLOCKPTR_BLOCK_SIZE */
#define BVEC_BLOCKPTR_BLOCK_SIZE	(8)

struct bvec
{
	u32 block_size;
	u32 element_size;
	u32 nelements;
	u32 free_elements;

	void **elements;
};

typedef struct bvec * bvec_t;

typedef u32 (*bvec_unary_predicate)(void *, u32);
typedef u32 (*bvec_binary_predicate)(void *, void *, u32);

u32 bvec_init(u32 block_size, ku32 element_size, bvec_t *v);
u32 bvec_destroy(bvec_t *v);

void *bvec_grow(bvec_t v);
void *bvec_get(bvec_t v, ku32 item);
u32 bvec_size(bvec_t v);
u32 bvec_iterate(bvec_t v, bvec_unary_predicate f, u32 x);

#endif

