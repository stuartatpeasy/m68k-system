/*



*/

#include <klibc/stdio.h>
#include <memory/slab.h>

#define SLAB_SIZE       (1024)


s32 slab_init(void *p, const u8 alloc_unit, slab_t *s)
{
    /*
        "alloc_unit" contains ceil(log2(object_size))

        Slab allocation bitmap consists of one bit per object, located at the start of the slab.
        The length of the bitmap is rounded up to the nearest 32 bits.
    */
    const u16 obj_len = 1 << alloc_unit;
    const u16 obj_len_bits = obj_len << 3;
    const u16 nobjs = (SLAB_SIZE >> alloc_unit);

    const u16 bitmap_bits_needed = (nobjs + 31) & ~31;
    const u16 bitmap_objs_needed = ((bitmap_bits_needed + (obj_len_bits - 1))
                                        & ~(obj_len_bits - 1)) >> (3 + alloc_unit);

    u16 u;
/*
    printf("Block allocator: p=%08x alloc_unit=%u\n"
           "obj_len=%u  nobjs=%u\n"
           "bitmap_bits_needed=%u  bitmap_objs_needed=%u\n\n",
           p, alloc_unit, obj_len, nobjs, bitmap_bits_needed, bitmap_objs_needed);
*/
    for(u = 0; u < bitmap_objs_needed; ++u)
        SLAB_BITMAP_SET_USED(p, u);

    for(; u < nobjs; ++u)
        SLAB_BITMAP_SET_FREE(p, u);

    for(; u < bitmap_bits_needed; ++u)
        SLAB_BITMAP_SET_USED(p, u);

    s->p = p;
    s->alloc_unit = alloc_unit;

    return SUCCESS;
}


void *slab_alloc(slab_t * const s)
{
    /* Compute length of the allocation bitmap in bytes */
    const u16 bm_nbytes = SLAB_SIZE >> (s->alloc_unit + 3);

    u8 *p_ = (u8 *) s->p;
    u16 u;
    for(u = 0; u < bm_nbytes; ++u)
    {
        if(p_[u] != 0xff)
        {
            u8 v, i;
            for(i = 0, v = 0x80; v; v >>= 1, ++i)
            {
                if(!(p_[u] & v))
                {
                    SLAB_BITMAP_SET_USED(s->p, (u << 3) + i);
                    return s->p + (((u << 3) + i) << s->alloc_unit);
                }
            }
        }
    }

    return NULL;
}


void slab_free(slab_t * const s, void *object)
{
    SLAB_BITMAP_SET_FREE(s->p, (object - s->p) >> s->alloc_unit);
}
