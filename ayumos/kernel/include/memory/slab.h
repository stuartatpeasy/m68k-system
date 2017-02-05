#ifndef KERNEL_INCLUDE_MEMORY_SLAB_H_INC
#define KERNEL_INCLUDE_MEMORY_SLAB_H_INC
/*
    Slab allocator

    Part of ayumos


    (c) Stuart Wallace, July 2015.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>


#define SLAB_MIN_RADIX  (1)     /* Smallest object size = 2^SLAB_MIN_RADIX      */
#define SLAB_MAX_RADIX  (6)     /* Largest object size = 2^SLAB_MAX_RADIX       */


#define SLAB_SIZE_LOG2  (10)
#define SLAB_SIZE       (1 << SLAB_SIZE_LOG2)


/* Round a u8 val in the range [1, 127] up to the next power of 2. */
#define ROUND_UP_PWR2(x)        \
    ({                          \
        u8 x_ = (x) - 1;        \
        x_ |= x_ >> 1;          \
        x_ |= x_ >> 2;          \
        x_ |= x_ >> 4;          \
        ++x_;                   \
    })


typedef struct slab_header slab_header_t;

struct slab_header
{
    slab_header_t *prev;
    slab_header_t *next;
    u16 free;
    u8 radix;
};


void slab_init();
s32 slab_create(ku8 radix, slab_header_t * const prev, slab_header_t **slab);
s32 slab_alloc(u8 size, void **p);
void slab_free(void *obj);
s32 slab_get_stats(ku8 radix, u32 *total, u32 *free);

#endif
