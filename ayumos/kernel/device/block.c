/*
    block.c: functions relating to the block device abstraction

    Part of ayumos


    (c) Stuart Wallace, November 2016.

    NOTE: this abstraction assumes a 512-byte block size.  This is likely to become a problem.
    NOTE: the block cache statistics object is not protected by locking.  The values stored in this
          object should therefore be regarded as approximate.
*/

#include <kernel/include/device/block.h>
#include <kernel/include/error.h>
#include <kernel/include/memory/kmalloc.h>
#include <kernel/include/semaphore.h>
#include <klibc/include/stdio.h>
#include <klibc/include/string.h>
#include <klibc/include/strings.h>


static block_cache_t bc;

u32 block_cache_get_slot(const dev_t *dev, ku32 block);


/*
    block_cache_init() - initialise a fixed-size block cache
*/
s32 block_cache_init(ku32 size)
{
    u32 i;

    bc.descriptors = (block_descriptor_t *) umalloc(size * sizeof(block_descriptor_t));
    if(!bc.descriptors)
        return -ENOMEM;

    bc.cache = (u8 *) umalloc(size * BLOCK_SIZE);
    if(!bc.cache)
    {
        ufree(bc.descriptors);
        return -ENOMEM;
    }

    for(i = 0; i < size; ++i)
    {
        bc.descriptors[i] = (block_descriptor_t)
        {
            .dev   = NULL,
            .block = 0,
            .flags = 0
        };

        sem_init(&bc.descriptors[i].sem);
    }

    bc.stats = (block_cache_stats_t) {0};
    bc.nblocks = size;

    printf("block cache: allocated %u bytes (%u blocks)\n", size * BLOCK_SIZE, size);
    return SUCCESS;
}


/*
    block_cache_get_slot() - return the slot in which a cached block must be stored.
*/
u32 block_cache_get_slot(const dev_t * const dev, ku32 block)
{
    return (((addr_t) dev ^ block) * GREAT_BIG_PRIME) % bc.nblocks;
}


/*
    block_read() - read a block, using the block cache.
*/
s32 block_read(dev_t * const dev, ku32 block, void *buf)
{
    block_descriptor_t *bd;
    void *data;
    u32 slot, one = 1;
    s32 ret;

    if(dev->type != DEV_TYPE_BLOCK)
        return -EINVAL;

    if(!bc.nblocks)
        return dev->read(dev, 0, &one, buf);

    slot = block_cache_get_slot(dev, block);
    bd = bc.descriptors + slot;
    data = bc.cache + (slot * BLOCK_SIZE);

    sem_acquire(&bd->sem);

    if((bd->dev != dev) || (bd->block != block))
    {
        if(bd->flags & BC_DIRTY)
        {
            /* Descriptor in use; block is dirty; evict it. */
            ret = bd->dev->write(bd->dev, bd->block, &one, (bd->flags & BC_ZERO) ? NULL : data);
            if(ret < 1)
            {
                sem_release(&bd->sem);
                return (ret == 0) ? -EWRITE : ret;
            }

            ++bc.stats.evictions;
            bd->flags = 0;
        }

        /* Read new block into slot */
        ret = dev->read(dev, block, &one, data);
        if(ret < 0)
        {
            sem_release(&bd->sem);
            return (ret == 0) ? -EREAD : ret;
        }

        /* Update descriptor */
        bd->dev = dev;
        bd->block = block;

        ++bc.stats.misses;
    }
    else
        ++bc.stats.hits;

    ++bc.stats.reads;

    if(bd->flags & BC_ZERO)
        bzero(buf, BLOCK_SIZE);
    else
        memcpy(buf, data, BLOCK_SIZE);

    sem_release(&bd->sem);

    return SUCCESS;
}


/*
    block_write() - write a block, via the block cache.
*/
s32 block_write(dev_t * const dev, ku32 block, const void * const buf)
{
    block_descriptor_t *bd;
    void *data;
    u32 slot, one = 1;
    s32 ret;

    if(dev->type != DEV_TYPE_BLOCK)
        return -EINVAL;

    if(!bc.nblocks)
    {
        ret = dev->write(dev, 0, &one, buf);
        if(ret < 1)
            return (ret == 0) ? -EWRITE : ret;

        return SUCCESS;
    }

    slot = block_cache_get_slot(dev, block);
    bd = bc.descriptors + slot;
    data = bc.cache + (slot * BLOCK_SIZE);

    sem_acquire(&bd->sem);

    if((bd->dev != dev) || (bd->block != block))
    {
        if(bd->flags & BC_DIRTY)
        {
            /* Descriptor in use; block is dirty; evict it. */
            ret = bd->dev->write(bd->dev, bd->block, &one, (bd->flags & BC_ZERO) ? NULL : data);
            if(ret < 1)
            {
                sem_release(&bd->sem);
                return (ret < 1) ? -EWRITE : ret;
            }

            ++bc.stats.evictions;
            bd->flags = 0;
        }

        ++bc.stats.misses;
    }
    else
        ++bc.stats.hits;

    /* Write the data to the device */
    ret = dev->write(dev, block, &one, buf);
    if(ret < 1)
    {
        sem_release(&bd->sem);
        return (ret == 0) ? -EWRITE : ret;
    }

    /* Copy the new block into the cache */
    if(buf != NULL)
        memcpy(data, buf, BLOCK_SIZE);

    /* Update descriptor */
    bd->dev = dev;
    bd->block = block;
    bd->flags = (buf == NULL) ? BC_ZERO : 0;

    ++bc.stats.writes;
    sem_release(&bd->sem);

    return SUCCESS;
}


/*
    block_read_multi() - read multiple blocks, using the block cache.
*/
s32 block_read_multi(dev_t * const dev, u32 block, u32 count, void *buf)
{
    s32 ret;
    u8 *p;
    u32 remaining;

    for(remaining = count, p = (u8 *) buf; remaining--; ++block, p += BLOCK_SIZE)
    {
        ret = block_read(dev, block, p);
        if(ret != SUCCESS)
            return ret;
    }

    return count;
}


/*
    block_write_multi() - write multiple blocks, using the block cache.
*/
s32 block_write_multi(dev_t * const dev, u32 block, u32 count, const void *buf)
{
    s32 ret;
    ku8 *p;
    u32 remaining;

    if(buf != NULL)
    {
        for(remaining = count, p = (ku8 *) buf; remaining--; ++block, p += BLOCK_SIZE)
        {
            ret = block_write(dev, block, p);
            if(ret != SUCCESS)
                return ret;
        }
    }
    else
    {
        /* Block should be zero-filled */
        for(remaining = count; remaining--;)
        {
            ret = block_write(dev, block++, NULL);
            if(ret != SUCCESS)
                return ret;
        }
    }

    return count;
}


/*
    block_cache_sync() - flush all dirty blocks.
*/
s32 block_cache_sync()
{
    block_descriptor_t *bd, * const end = bc.descriptors + bc.nblocks;
    u32 one = 1;
    s32 ret;

    for(bd = bc.descriptors; bd != end; ++bd)
    {
        sem_acquire(&bd->sem);

        if(bd->flags & BC_DIRTY)
        {
            void *data = bc.cache + ((bd - bc.descriptors) * BLOCK_SIZE);
            ret = bd->dev->write(bd->dev, bd->block, &one, data);
            if(ret != SUCCESS)
                return ret;

            bd->flags &= ~BC_DIRTY;
        }

        sem_release(&bd->sem);
    }

    return SUCCESS;
}


/*
    block_cache_stats() - retrieve block cache statistics
*/
const block_cache_stats_t *block_cache_stats()
{
    return &bc.stats;
}
