#ifndef KERNEL_DEVICE_BLOCK_H_INC
#define KERNEL_DEVICE_BLOCK_H_INC
/*
	Block device generic functions and declarations

	Part of ayumos


	Stuart Wallace, August 2015.
*/

#include <kernel/include/defs.h>
#include <kernel/include/device/device.h>
#include <kernel/include/types.h>
#include <kernel/include/semaphore.h>

#define GREAT_BIG_PRIME             (0xfffffffb)    /* Largest prime representable as a u32 */

/* Cached-block flags */
#define BC_DIRTY                    BIT(1)  /* Block has been modified                          */
#define BC_LOCKED                   BIT(2)  /* Block is locked in cache (cannot be evicted)     */


typedef u32 block_id;


/* Block descriptor - reflects the status of a single block in the block cache */
typedef struct block_descriptor
{
    dev_t       *dev;       /* Device containing the block              */
    block_id    block;      /* ID of the block on the device            */
    u16         flags;      /* Information associated with the block    */
    sem_t       sem;        /* Semaphore for locking the block          */
} block_descriptor_t;


/* Block cache statistics */
typedef struct block_cache_stats
{
    u32 reads;
    u32 writes;
    u32 evictions;
    u32 hits;
    u32 misses;
} block_cache_stats_t;


/* Block cache metadata */
typedef struct block_cache
{
    block_descriptor_t *descriptors;
    u8 *cache;
    u32 nblocks;
    block_cache_stats_t stats;
} block_cache_t;


typedef struct blockdev_stats
{
    u32 blocks_read;
    u32 blocks_written;
} blockdev_stats_t;


s32 block_cache_init(ku32 size);
s32 block_read(dev_t * const dev, ku32 block, void *buf);
s32 block_read_multi(dev_t * const dev, u32 block, u32 count, void *buf);
s32 block_cache_sync();
const block_cache_stats_t *block_cache_stats();

#endif
