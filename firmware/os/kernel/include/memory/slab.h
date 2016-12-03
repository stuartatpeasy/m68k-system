#ifndef KERNEL_MEMORY_SLAB_H_INC
#define KERNEL_MEMORY_SLAB_H_INC
/*
    Slab allocator

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, July 2015.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>


#define NSLABS          (8)
#define SLAB_SIZE       (1024)
#define SLAB_SIZE_LOG2  (10)

#if DATA_BUS_WIDTH == 8
#define SLAB_BITMAP_SET_USED(p, u) \
    (*(((u8 *) (p)) + ((u) >> 3)) |= (0x80 >> ((u) & 7)))
#define SLAB_BITMAP_SET_FREE(p, u) \
    (*(((u8 *) (p)) + ((u) >> 3)) &= ~(0x80 >> ((u) & 7)))
#elif DATA_BUS_WIDTH == 16
#define SLAB_BITMAP_SET_USED(p, u) \
    (*(((u16 *) (p)) + ((u) >> 4)) |= (0x8000 >> ((u) & 15)))
#define SLAB_BITMAP_SET_FREE(p, u) \
    (*(((u16 *) (p)) + ((u) >> 4)) &= ~(0x8000 >> ((u) & 15)))
#else
#define SLAB_BITMAP_SET_USED(p, u) \
    (*(((u32 *) (p)) + ((u) >> 5)) |= (0x80000000 >> ((u) & 31)))
#define SLAB_BITMAP_SET_FREE(p, u) \
    (*(((u32 *) (p)) + ((u) >> 5)) &= ~(0x80000000 >> ((u) & 31)))
#endif

#define ROUND_UP_PWR2(x)        \
    ({                          \
        u8 x_ = (x) - 1;        \
        x_ |= x_ >> 1;          \
        x_ |= x_ >> 2;          \
        x_ |= x_ >> 4;          \
        ++x_;                   \
    })


struct slab
{
    void *p;
    u8 alloc_unit;
};

typedef struct slab slab_t;

slab_t g_slabs[NSLABS];
void *g_slab_base;
void *g_slab_end;

void *slab_alloc_pow2(ku32 radix);
void *slab_alloc_obj(slab_t * const s);
s32 slab_create(void *p, u8 const alloc_unit, slab_t *s);
void slab_free(void *obj);
void slab_init(void *slab_base);

#endif
