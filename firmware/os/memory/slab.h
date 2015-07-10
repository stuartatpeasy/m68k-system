#ifndef MEMORY_BLOCKALLOC_H_INC
#define MEMORY_BLOCKALLOC_H_INC
/*

*/

#include <include/defs.h>
#include <include/types.h>


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

struct slab
{
    void *p;
    u8 alloc_unit;
};

typedef struct slab slab_t;

s32 slab_init(void *p, u8 const alloc_unit, slab_t *s);
void *slab_alloc(slab_t * const s);
void slab_free(slab_t * const s, void *object);

#endif // MEMORY_BLOCKALLOC_H_INC
