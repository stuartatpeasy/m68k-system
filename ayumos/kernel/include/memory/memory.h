#ifndef KERNEL_INCLUDE_MEMORY_MEMORY_H_INC
#define KERNEL_INCLUDE_MEMORY_MEMORY_H_INC
/*
    Kernel memory management

    Part of ayumos


    (c) Stuart Wallace, December 2016.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>

/*
    Kernel memory block size can be overridden by the architecture, or the platform.  It will
    probably end up being equal to PAGE_SIZE on systems with virtual memory.
*/
#ifndef KERNEL_MEM_BLOCK_SIZE
#define KERNEL_MEM_BLOCK_SIZE   (1024)
#endif

/* Block types */
#define BLOCK_TYPE_EMPTY        (0)     /* Unused block                 */
#define BLOCK_TYPE_SLAB         (1)     /* Block allocated as a slab    */
#define BLOCK_TYPE_CACHE_BLOCK  (2)
#define BLOCK_TYPE_CACHE_DENTRY (3)
#define BLOCK_TYPE_STACK        (4)     /* Per-process kernel stack block                       */
#define BLOCK_TYPE_GENERAL      (5)     /* General-use block, for when slabs aren't appropriate */

/* Block allocation flags */
#define MBF_NONE                (0)
#define MBF_ZERO                BIT(0)  /* Zero the block before granting it to the requester   */
#define MBF_GUARD               BIT(1)  /* Fill block with "guard" data e.g. to detect overflow */


typedef u8 kmem_block_type_t;


typedef struct kmem_block
{
    void *p;                /* Pointer to the block in memory           */
    u16 info;               /* Data/flags associated with the block     */
    kmem_block_type_t type;
} kmem_block_t;


#endif
