/*
    Functions relating to the manipulation of memory extents, which are blocks of memory.

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.
*/

#include <kernel/memory/extents.h>


/*
    mem_get_largest_extent() - get the largest extent matching flags
*/
mem_extent_t *mem_get_largest_extent(ku32 flags)
{
    mem_extent_t *e, *ret = NULL;
    u32 sz = 0;

    for_each_matching_mem_extent(e, flags)
        if(e->len > sz)
        {
            sz = e->len;
            ret = e;
        }

    return ret;
}


/*
    mem_zero_extents() - zero the extents matching flags
*/
void mem_zero_extents(ku32 flags)
{
    mem_extent_t *e;

    for_each_matching_mem_extent(e, flags)
        bzero(e->base, e->len);
}


/*
    mem_get_total_size() - return the total size of matching extents
*/
u32 mem_get_total_size(ku32 flags)
{
    mem_extent_t *e;
    u32 sz = 0;

    for_each_matching_mem_extent(e, flags)
        sz += e->len;

    return sz;
}


/*
    mem_get_highest_addr() - return the highest address in any matching extent
*/
void *mem_get_highest_addr(ku32 flags)
{
    mem_extent_t *e;
    void *highest = 0;

    for_each_matching_mem_extent(e, flags)
        if((e->base + e->len) > highest)
            highest = e->base + e->len;

    return highest;
}


/*
    mem_get_containing_extent() - return the extent containing address addr
*/
mem_extent_t *mem_get_containing_extent(void *addr)
{
    mem_extent_t *e;

    for_each_mem_extent(e)
        if((addr > e->base) && (addr < (void *) ((u8 *) e->base + e->len)))
            return e;

    return NULL;
}
