/*
	heap.c: simple heap memory allocator

	Part of the as-yet-unnamed MC68010 operating system.

	This module implements a very simple linked-list-based heap memory allocator.  It provides a
	back-end for the libc functions malloc, calloc, realloc and free.  All the functions below take
	as their first argument a pointer to a struct os_heap.  This allows separate heaps to be
	maintained - perhaps one for supervisor-mode code and one for user-mode applications, and maybe
	one per user-mode application.

	Each allocated memory block consists of a header (a struct os_memblock) and a region of memory
	which may be used by client code.  The header contains a magic number and a "size" field.  The
	top	31 bits of the magic number store a meaningless but identifiable bit pattern
	(MEMBLOCK_HDR_MAGIC) and the least-significant bit acts as an "in-use" flag.  The "size" field
	is an unsigned integer specifying the number of bytes allocated, excluding the size of the
	header.  At first sight it appears illogical to use a "size" fields instead of a pointer to the
	next block; however in this specific application it turns out that the pointer arithmetic is no
	simpler in either case.

	Memory is allocated in aligned blocks.  The alignment is specified by the MEMBLOCK_ALIGN macro;
	allocation requests will be rounded up such that the allocation ends on an alignment boundary.
	The value of the MEMBLOCK_ALIGN macro indicates the power to which two must be raised in order
	to obtain the alignment boundary.  For example, if MEMBLOCK_ALIGN equals 2, allocations will be
	performed against a (1 << 2) = four-byte boundary and a request for 17 bytes will cause 20
	bytes to be allocated.  Macros are used in the os_calloc() and os_realloc() functions to vary
	their block-copying/-clearing loop word size in accordance with the value of MEMBLOCK_ALIGN.

	The os_free() function attempts to minimise heap fragmentation by combining adjacent free blocks
	to form a single, larger block.  Whenever a block is freed with os_free(), the next block is
	checked.  If it is also free, the two are combined.  This test is repeated until an in-use block
	is found or the end of the heap is reached.  An optimisation is possible here: the function
	could also check blocks before the freed block.  To be efficient this might require a change to
	the os_memblock structure; at present it implements a singly-linked list and there is therefore
	no efficent way to traverse back to previous blocks.


	(c) Stuart Wallace, 13th August 2011.
*/

#include <kernel/memory/heap.h>
#include <klibc/stdio.h>


#define MEMBLOCK_HDR_MAGIC	(0xc91d58be)	/* meaningless number used as a signature to identify a
											   memory block; important that the bottom bit isn't set
											   (this is used as the free / in-use flag; 0 = free) */

#define MEMBLOCK_ALIGN 		(2)				/* all blocks will be allocated on 2^MEMBLOCK_ALIGN-byte
											   boundaries */

											/* Mask used to align numbers to MEMBLOCK_ALIGN
											   boundaries. */
#define MEMBLOCK_ALIGN_MASK ((1 << MEMBLOCK_ALIGN) - 1)

/*
	Header for allocated memory blocks.  Note that the size field specifies the size of the block
	excluding the size of the header structure.
*/
typedef struct heap_memblock_
{
	unsigned int	magic;
	unsigned int	size;
} heap_memblock;


/*
	heap_init(): initialise the heap.  This must be called before either heap_malloc() or
	heap_free() are used.
*/
void heap_init(heap_ctx * const heap, void * const mem, u32 mem_len)
{
	mem_len &= ~0x1;	/* Round mem_len down to a multiple of 2 */

	heap->start = mem;
	heap->size = mem_len;

	heap_memblock *p = (heap_memblock *) heap->start;

	p->magic = MEMBLOCK_HDR_MAGIC;
	p->size = heap->size - (2 * sizeof(heap_memblock));

	/* Create end-of-heap marker */
	p = (heap_memblock *) ((u8 *) heap->start + p->size + sizeof(heap_memblock));
	
	p->magic = MEMBLOCK_HDR_MAGIC | 0x1;	/* Mark end-of-heap block as used */
	p->size = 0;
}


/*
	heap_malloc(): allocate heap memory.  Returns pointer to allocated block, or 0 (NULL) on
	failure.
*/
void *heap_malloc(const heap_ctx * const heap, u32 size)
{
	heap_memblock *p = (heap_memblock *) heap->start, *p2;

	if(!size)
		return 0;

	size = (size + MEMBLOCK_ALIGN_MASK) & ~MEMBLOCK_ALIGN_MASK;

	while(p->size)
	{
		/* Is this block free and large enough? */
		if(!(p->magic & 0x1) && (size <= p->size))
		{
			/* Is it big enough to be subdivided (ie. is there room for another allocation
			   between it and the next block)?  If not, allocate the entire block. */
			if((p->size - size) > (sizeof(heap_memblock) + MEMBLOCK_ALIGN_MASK))
			{
				/* Divide the block */
				p2 = (heap_memblock *) ((char *) (p + 1) + size);
				p2->magic = MEMBLOCK_HDR_MAGIC;
				p2->size = p->size - (size + sizeof(heap_memblock));

				p->size = size;
			}

			p->magic |= 0x1;		/* Mark the block as allocated */
			return (void *) ++p;
		}
		p = (heap_memblock *) ((unsigned char *) p + p->size + sizeof(heap_memblock));
	}
	return 0;		/* Reached the end of the heap without finding a free block. */
}


/*
	heap_calloc(): allocate and clear heap memory.  Returns a pointer to the allocated and cleared
	memory or 0 (NULL) on failure.
*/
void *heap_calloc(const heap_ctx * const heap, ku32 nmemb, ku32 size)
{
	u32 n = nmemb * size;
	void *p = heap_malloc(heap, n);

	if(p)
    {
        n += (1 << MEMBLOCK_ALIGN) - 1;
        for(n >>= MEMBLOCK_ALIGN; n--;)
        {
#if(MEMBLOCK_ALIGN == 2)
            ((u32 *) p)[n] = 0;
#elif(MEMBLOCK_ALIGN == 1)
            ((u16 *) p)[n] = 0;
#elif(MEMBLOCK_ALIGN == 0)
            ((u8 *) p)[n] = 0;
#else
#error "Invalid MEMBLOCK_ALIGN constant value"
#endif
        }
    }

	return p;
}


/*
	heap_realloc(): change the size of the memory block at ptr.  Copy contents to the new memory
	block to the maximum extent possible.  Newly allocated memory will be uninitialised.
*/
void *heap_realloc(const heap_ctx * const heap, const void *ptr, u32 size)
{
	heap_memblock *p = (heap_memblock *) ((u8 *) ptr - sizeof(heap_memblock));
	void *pnew;

	/* If the new block size is zero and the original block pointer is non-NULL, this call is
	   equivalent to free(ptr). */
	if(!size && ptr)
	{
		heap_free(heap, ptr);
		return 0;
	}

	/* If ptr is 0 and size is non-zero, the call is equivalent to malloc(size) */
	if(!ptr && size)
		return heap_malloc(heap, size);

	if(p->magic == (MEMBLOCK_HDR_MAGIC | 0x1))
	{
		if(!(pnew = heap_malloc(heap, size)))
			return 0;			/* New block allocation failed */

		if(size > p->size)
			size = p->size;

        size += (1 << MEMBLOCK_ALIGN) - 1;

		for(size >>= MEMBLOCK_ALIGN; size--;)
        {
#if(MEMBLOCK_ALIGN == 2)
			((u32 *) pnew)[size] = ((u32 *) ptr)[size];
#elif(MEMBLOCK_ALIGN == 1)
			((u16 *) pnew)[size] = ((u16 *) ptr)[size];
#elif(MEMBLOCK_ALIGN == 0)
			((u8 *) pnew)[size] = ((u8 *) ptr)[size];
#else
#error "Invalid MEMBLOCK_ALIGN constant value"
#endif
        }

		heap_free(heap, ptr);
		return pnew;
	}
	else
	{
#ifdef DEBUG_KMALLOC
		printf("heap_realloc(%p, %u): not allocated\n", ptr, size);
#endif
		return 0;
	}
}


/*
	heap_free(): free memory allocated with heap_malloc().
*/
void heap_free(const heap_ctx * const heap, const void *ptr)
{
	heap_memblock *p = (heap_memblock *) ((u32) ptr - sizeof(heap_memblock)),
				  *p_;

	if(!ptr)
		return;		/* Take no action if ptr is NULL. */

	if(p->magic == (MEMBLOCK_HDR_MAGIC | 1))
	{
#ifdef DEBUG_KMALLOC
		/* Check that the next block is intact */
		const heap_memblock * const p_next = (heap_memblock *) ((u8 *) ptr + p->size);
		if((p_next->magic & ~0x1) != MEMBLOCK_HDR_MAGIC)
			printf("heap_free(%p): block (size %d) wrote beyond bounds\n", ptr, p->size);
#endif
		p->magic &= ~1;		/* Mark block free */

		/* Merge any subsequent free blocks into this block */
		for(p_ = (heap_memblock *) ((u8 *) ptr + p->size);
			(p_ < (heap_memblock *) (heap->start + heap->size)) && !(p_->magic & 0x1);
			p_ = (heap_memblock *) (p_->size + (u8 *) (p_ + 1)))
		{
			p->size += p_->size + sizeof(heap_memblock);
		}
		/* TODO: also merge this block into previous free blocks */
	}
#ifdef DEBUG_KMALLOC
	else if(p->magic == MEMBLOCK_HDR_MAGIC)
		printf("heap_free(%p): double-free\n", ptr);
	else
        printf("heap_free(%p): not allocated\n", ptr);
#endif
}


/*
	heap_freemem(): return the number of free bytes in the specified heap.  Note that it may not
	be possible to allocate a single block of this size because of fragmentation.
*/
u32 heap_freemem(const heap_ctx * const heap)
{
	heap_memblock *p = (heap_memblock *) heap->start;
	u32 free = 0;

	while(p < (heap_memblock *) (heap->start + heap->size))
	{
		if(p->magic == MEMBLOCK_HDR_MAGIC)
			free += p->size;

		p = (heap_memblock *) ((u8 *) p + p->size + sizeof(heap_memblock));
	}

	return free;
}


/*
	heap_usedmem(): return the number of allocated bytes in the specified heap.  Note that this
	figure does not include overhead.
*/
u32 heap_usedmem(const heap_ctx * const heap)
{
	heap_memblock *p = (heap_memblock *) heap->start;
	u32 used = 0;

	while(p < (heap_memblock *) (heap->start + heap->size))
	{
		if(p->magic == (MEMBLOCK_HDR_MAGIC | 1))
			used += p->size;

		p = (heap_memblock *) ((u8 *) p + p->size + sizeof(heap_memblock));
	}

	return used;
}

