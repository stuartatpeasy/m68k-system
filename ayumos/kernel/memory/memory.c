/*
    Kernel memory management

    Part of ayumos


    (c) Stuart Wallace, December 2016.
*/

#include <kernel/include/memory/memory.h>


static kmem_block_t *block_map; /* The block allocation array                   */
static u32 nblocks;             /* The total number of blocks under management  */


/*
    mem_init() - initialise kernel memory manager
*/
s32 mem_init(void *start, ku32 len)
{
    u32 i, block_map_len;

    /* Initialise memory: set up kmem_block_t array; mark all blocks as unused */
    nblocks = len / (KERNEL_MEM_BLOCK_SIZE + sizeof(kmem_block_t));

    block_map_len = ((nblocks * sizeof(kmem_block_t)) + (KERNEL_MEM_BLOCK_SIZE - 1)) /
                        KERNEL_MEM_BLOCK_SIZE;

    /* Note that we're not actually "allocating" space here */
    block_map = (kmem_block_t *) start;

    for(i = 0; i < nblocks; ++i)
        blocks[i] =
}


/*
    mem_alloc_block() - allocate
*/
void *mem_alloc_block(const kmem_block_type_t type, u32 flags)
{

}
