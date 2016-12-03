/*
    Slab allocator

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, July 2015.
*/

#include <kernel/include/memory/slab.h>


slab_t g_slabs[NSLABS];
void *g_slab_base;
void *g_slab_end;


/*
    slab_init() - initialise the slab allocator and create some initial slabs.
*/
void slab_init(void *slab_base)
{
    u8 u;

    g_slab_base = slab_base;

    for(u = 0; u < NSLABS; ++u, slab_base += SLAB_SIZE)
        slab_create(slab_base, u + 1, &(g_slabs[u]));

    g_slab_end = slab_base;
}


/*
    slab_alloc_pow2() - allocate an object of size 2^radix bytes in an appropriate slab.
*/
void *slab_alloc_pow2(ku32 radix)
{
    if(!radix)
        return NULL;    /* Requested allocation is 0 */

    return slab_alloc_obj(&(g_slabs[radix - 1]));
}


/*
    slab_free() - free an object
*/
void slab_free(void *obj)
{
    const slab_t *s = &(g_slabs[(obj - g_slab_base) >> SLAB_SIZE_LOG2]);
    SLAB_BITMAP_SET_FREE(s->p, (obj - s->p) >> s->alloc_unit);
}


/*
    slab_free_obj() - free an object in the specified slab.
*/
void slab_free_obj(slab_t * const s, void *object)
{
    SLAB_BITMAP_SET_FREE(s->p, (object - s->p) >> s->alloc_unit);
}


/*
    slab_create() - create a slab of size in the memory pointed to by p, which should be a region
    SLAB_SIZE bytes in length, and initialise it to hold objects of size 2^alloc_unit.  The slab
    object data is written to s.
*/
s32 slab_create(void *p, const u8 alloc_unit, slab_t *s)
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


/*
    slab_alloc_obj() - allocate an object within slab s, and return a pointer to it.  Return NULL if
    the slab is full.
*/
void *slab_alloc_obj(slab_t * const s)
{
    /* Compute length of the allocation bitmap in halfwords */
    const u16 nwords = SLAB_SIZE >> (s->alloc_unit + 5);

    u16 u;
    for(u = 0; u < nwords; ++u)
    {
        u32 bits = ((u32 *) (s->p))[u];
        if(bits != 0xffffffff)
        {
            /* Find first cleared bit in bits - this will yield the first available block */
            u16 objnum, i = 0;

            if((bits & 0xffff0000) != 0xffff0000)
            {
                bits >>= 16;    /* Free block is in bits 31-16 */
                i += 16;
            }

            if((bits & 0x0000ff00) != 0x0000ff00)
            {
                bits >>= 8;     /* Free block is in bits 15-8 */
                i += 8;
            }

            if((bits & 0x000000f0) != 0x000000f0)
            {
                bits >>= 4;     /* Free block is in bits 7-4 */
                i += 4;
            }

            if((bits & 0x0000000c) != 0x0000000c)
            {
                bits >>= 2;     /* Free block is in bits 3-2 */
                i += 2;
            }

            if((bits & 0x00000002) != 0x00000002)
                i += 1;         /* Free block is in bit 1 */

            objnum = (u << 5) + (31 - i);
            SLAB_BITMAP_SET_USED(s->p, objnum);

            return s->p + (objnum << s->alloc_unit);
        }
    }

    return NULL;
}
