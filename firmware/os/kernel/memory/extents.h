#ifndef KERNEL_MEMORY_EXTENTS_H_INC
#define KERNEL_MEMORY_EXTENTS_H_INC
/*
    Declarations relating to the manipulation of memory extents, which are blocks of memory.

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, September 2015.
*/

#include <include/defs.h>
#include <include/types.h>
#include <klibc/strings.h>

#define MEM_EXTENT_USER     (0x00000001)    /* User-accessible memory                       */
#define MEM_EXTENT_KERN     (0x00000002)    /* Kernel mem - access violation on user access */

#define MEM_EXTENT_RAM      (0x00000100)    /* Read/write memory                            */
#define MEM_EXTENT_ROM      (0x00000200)    /* Read-only memory                             */
#define MEM_EXTENT_PERIPH   (0x00000400)    /* Memory-mapped peripherals                    */
#define MEM_EXTENT_VACANT   (0x00000800)    /* Vacant extent - nothing maps here            */

#define MEM_EXTENT_MASK_ANY (0xffffffff)    /* Match any kind of extent                     */

typedef struct mem_extent
{
    void *      base;
    u32         len;
    u32         flags;
} mem_extent_t;

/* These will be set by platform-specific code at boot time */
mem_extent_t *g_mem_extents;
mem_extent_t *g_mem_extents_end;

/* Iterate over all memory extents */
#define for_each_mem_extent(p)                                              \
    for(p = g_mem_extents; p < g_mem_extents_end; ++p)

/* Iterate over memory extents whose flags match "fl" */
#define for_each_matching_mem_extent(p, fl)                                 \
    for_each_mem_extent(p)                                                  \
        if(((fl) == MEM_EXTENT_MASK_ANY) || ((p->flags & (fl)) == (fl)))

mem_extent_t *mem_get_largest_extent(ku32 flags);
void mem_zero_extents(ku32 flags);
u32 mem_get_total_size(ku32 flags);
void *mem_get_highest_addr(ku32 flags);
mem_extent_t *mem_get_containing_extent(void *addr);


#endif
