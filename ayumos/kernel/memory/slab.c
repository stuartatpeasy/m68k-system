/*
    Slab allocator

    Part of ayumos

    A slab is a fixed-length block of memory.  It consists of a header, an allocation bitmap, and
    an array of equal-sized "objects".  The intent of slabs is to provide a means to allocate small
    units of memory with minimal overhead.

    The size of the objects in a slab is always a power of two. An object can be allocated by
    calling slab_alloc(), which will locate (or create) a slab containing objects of an appropriate
    size, mark an object as "in use" in the slab's allocation bitmap, and return a pointer to the
    object.

    Slabs are organised into linked lists according to their "radix", i.e. log2(object_size).  The
    constants SLAB_MIN_RADIX and SLAB_MAX_RADIX define the limits of object size.  The current
    implementation limits the maximum object size to 64 bytes (i.e. SLAB_MAX_RADIX<=6).


    (c) Stuart Wallace, July 2015.
*/

#include <kernel/include/memory/slab.h>
#include <kernel/include/preempt.h>
#include <klibc/include/stdlib.h>

#ifdef DEBUG_KMALLOC
#include <klibc/include/stdio.h>
#endif


void *g_slab_start;     /* Pointer to the memory reserved for slabs                         */
void *g_slab_next;      /* Pointer to the next memory location available for slab creation  */
void *g_slab_end;       /* Pointer to the first byte after the end of the slab region       */

/*
    g_slabs is an array of linked lists of slab_t objects, indexed by slab radix (i.e. allocation
    unit size).  When an allocation is requested, the list corresponding to the allocation unit size
    is searched for a free block.  If no free block exists, a new slab will be initialised and
    added to the list.
*/
slab_header_t *g_slabs[(SLAB_MAX_RADIX - SLAB_MIN_RADIX) + 1] = {0};


/*
    slab_init() - initialise the slab allocator by setting the list head pointers in g_slabs[] to
    NULL.  Note that the kernel heap allocator may not be available when this function is called.
*/
void slab_init(void *start, u32 len)
{
    u8 u;

    g_slab_start = start;
    g_slab_next = start;
    g_slab_end = (void *) ((u8 *) start + len);

    for(u = 0; u < ARRAY_COUNT(g_slabs); ++u)
        g_slabs[u] = NULL;
}


/*
    slab_create() - allocate and initialise a new slab to hold objects of size 2^<radix>.  Append it
    to the slab pointed to by <prev>; return a pointer to the new slab through <slab>.
*/
s32 slab_create(ku8 radix, slab_header_t * const prev, slab_header_t **slab)
{
    slab_header_t *hdr;
    u16 nobjs, reserved_objs, bitmap_len_bytes, free;
    u8 *bitmap;

    if((radix < SLAB_MIN_RADIX) || (radix > SLAB_MAX_RADIX))
        return EINVAL;

    /* Allocate the entire slab, and obtain a pointer to the header */
    if(g_slab_next >= g_slab_end)
        return ENOMEM;

    hdr = (slab_header_t *) g_slab_next;
    g_slab_next = (u8 *) g_slab_next + SLAB_SIZE;

    /*
        The allocation bitmap will contain one bit for each (1 << radix) bytes in the slab, rounded
        up to the nearest multiple of 32 bits.  There are fewer usable objects than this in the
        slab, because the slab_header and the allocation bitmap itself consume space at the start of
        the slab, and the rounding-up of the allocation bitmap length may result in the bitmap
        representing objects past the end of the slab.  The "objects" occupied by the slab header
        and the bitmap, and those past the end of the slab, are therefore marked as "in use" when
        the slab is created.
    */
    nobjs = 1 << (SLAB_SIZE_LOG2 - radix);
    bitmap_len_bytes = (nobjs + 7) >> 3;

    /* Obtain a pointer to the start of the allocation bitmap */
    bitmap = (u8 *) (hdr + 1);

    /*
        Calculate the number of "reserved" objects at the start of the slab, rounding up as
        necessary, and mark the objects as "in use" (1).
    */
    reserved_objs = ((sizeof(slab_header_t) + bitmap_len_bytes) + ((1 << radix) - 1)) >> radix;

    free = nobjs - reserved_objs;

    for(; reserved_objs > 8; reserved_objs -= 8)
        *bitmap++ = 0xff;

    *bitmap = (1 << reserved_objs) - 1;

    /* Calculate the number of "reserved" objects at the end of the slab */
    reserved_objs = (bitmap_len_bytes * 8) - nobjs;
    free -= reserved_objs;

    /* Obtain a pointer to the last byte of the allocation bitmap */
    bitmap = ((u8 *) (hdr + 1)) + (bitmap_len_bytes - 1);

    /* Mark the "reserved" objects at the end of the slab as "in use" */
    for(; reserved_objs > 8; reserved_objs -= 8)
        *bitmap-- = 0xff;

    *bitmap = ~(((u8) 0xff) >> reserved_objs);

    /* Set up the header object */
    hdr->next = NULL;
    hdr->free = free;
    hdr->radix = radix;

    if(prev != NULL)
        prev->next = hdr;

    *slab = hdr;

    return SUCCESS;
}


/*
    slab_alloc() - choose a slab containing objects of the correct radix to store <size> bytes (i.e.
    round <size> up to the nearest power of two, and find a slab with that radix) and allocate an
    object.  This may result in the creation of a new slab.  Fail if the calculated radix is outside
    [SLAB_MIN_RADIX, SLAB_MAX_RADIX], or if the creation of a new slab fails.  Returns a pointer to
    the new object.
*/
void *slab_alloc(size_t size)
{
    s32 ret;
    u8 radix, *bitmap, bit;
    u16 obj;
    slab_header_t *slab;

    /* Zero-byte allocations are allowed in malloc(), so they're allowed here too. */
    if(!size)
        return NULL;

    if(size > (1 << SLAB_MAX_RADIX))
        return NULL;        /* Can't allocate that much space in a slab */

    if(size < (1 << SLAB_MIN_RADIX))
        size = 1 << SLAB_MIN_RADIX;
    else
        size = ROUND_UP_PWR2(size);

    /* Calculate log2(size) */
    for(size >>= 1, radix = 0; size; size >>= 1, ++radix)
        ;

    slab = g_slabs[radix - SLAB_MIN_RADIX];

    preempt_disable();      /* BEGIN locked section */

    if(slab == NULL)
    {
        /* No slab of the required size exists yet.  Create one. */
        ret = slab_create(radix, NULL, &g_slabs[radix - SLAB_MIN_RADIX]);
        if(ret != SUCCESS)
        {
            preempt_enable();
            return NULL;
        }

        slab = g_slabs[radix - SLAB_MIN_RADIX];
    }
    else
    {
        /* Walk the list of slabs of the appropriate size, looking for one that isn't full */
        for(; !slab->free; slab = slab->next)
        {
            if(slab->next == NULL)
            {
                /* Create a new slab and append it to the list */
                ret = slab_create(radix, slab, &slab->next);
                if(ret != SUCCESS)
                {
                    /*
                        Slab creation failed.  Given that we have already validated radix, this must
                        be an ENOMEM condition, hence we can safely return NULL.
                    */
                    preempt_enable();
                    return NULL;
                }
            }
        }
    }

    /* At this point <slab> points to a non-full slab.  Find a free entry. */
    bitmap = (u8 *) (slab + 1);

    /* Fast-walk past groups of 8 allocated objects */
    for(obj = 0, bitmap = (u8 *) (slab + 1); *bitmap == 0xff; ++bitmap, obj += 8)
        ;

    /* The current byte of the bitmap contains an unallocated object; find it */
    for(bit = 1; *bitmap & bit; bit <<= 1, ++obj)
        ;

    /* <obj> now contains an "object number", i.e. the offset into a slab of a free object */
    *bitmap |= bit;                                     /* Mark the object as allocated    */
    --slab->free;

    preempt_enable();       /* END locked section */

    return (void *) (((u8 *) slab) + (obj << radix));   /* Obtain a pointer to the object  */
}


/*
    slab_free() - free an object.
*/
void slab_free(void *obj)
{
    u16 offset;
    u8 *bitmap, bit;

    /* free(NULL) is allowed, so slab_free(NULL) is allowed too. */
    if(obj == NULL)
        return;

    /* Find the slab corresponding to this object */
    slab_header_t *slab = (slab_header_t *) ((u32) obj & ~(SLAB_SIZE - 1));

    offset = ((u32) obj & (SLAB_SIZE - 1)) >> slab->radix;

    bitmap = ((u8 *) (slab + 1)) + (offset >> 3);
    bit = 1 << (offset & 0x7);

    if(*bitmap & bit)
    {
        *bitmap &= ~bit;        /* Mark the object as free */
        ++slab->free;
    }
#ifdef DEBUG_KMALLOC
    else
        printf("slab_free(%p): double-free\n", obj);
#endif
}


/*
    slab_get_stats() - get statistics relating to all slabs of the specified radix.  <total> is set
    to the total number of objects (allocated or free) of the specified radix; <free> is set to the
    number of free objects of the specified radix.
*/
s32 slab_get_stats(ku8 radix, u32 *total, u32 *free)
{
    slab_header_t *slab;

    if((radix < SLAB_MIN_RADIX) || (radix > SLAB_MAX_RADIX))
        return EINVAL;

    *total = 0;
    *free = 0;

    for(slab = g_slabs[radix - SLAB_MIN_RADIX]; slab != NULL; slab = slab->next)
    {
        ku16 nobjs = 1 << (SLAB_SIZE_LOG2 - radix);
        ku16 bitmap_len_bytes = (nobjs + 7) >> 3;
        ku16 reserved_objs = (((sizeof(slab_header_t) + bitmap_len_bytes)
                                    + ((1 << radix) - 1)) >> radix)
                                + ((bitmap_len_bytes * 8) - nobjs);

        *total += nobjs - reserved_objs;
        *free += slab->free;
    }

    return SUCCESS;
}
