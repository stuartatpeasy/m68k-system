#ifndef KERNEL_INCLUDE_MEMORY_PRIMITIVES_H_INC
#define KERNEL_INCLUDE_MEMORY_PRIMITIVES_H_INC
/*
    Memory-operation "primitives".  These are small functions performing common memory-manipulation
    tasks.  The can be overridden by e.g. target-specific assembly-language versions in order to
    increase performance.

    Part of ayumos

    (c) Stuart Wallace <stuartw@atom.net>, March 2017.
*/

#include <kernel/include/types.h>
#include <kernel/include/cpu.h>


/* Byte-/word-swapping primitves */


/*
    bswap_16() - swap the endianness of a 16-bit half-word.
*/
#ifndef HAVE_bswap_16
#define HAVE_bswap_16
inline u16 bswap_16(u16 x)
{
    return (x << 8) | (x >> 8);
}
#endif


/*
    bswap_32() - swap the endianness of a 32-bit word.
*/
#ifndef HAVE_bswap_32
#define HAVE_bswap_32
inline u32 bswap_32(u32 x)
{
    return (x << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | (x >> 24);
};
#endif


/*
    wswap_32() - swap the 16-bit half-words in a 32-bit word.
*/
#ifndef HAVE_wswap_32
#define HAVE_wswap_32
inline u32 wswap_32(u32 x)
{
    return (x << 16) | (x >> 16);
};
#endif


/* Memory copying / zeroing primitives */

/*
    memcpy_align4() - copy <count> bytes from <src> to <dest>, with the assumption that both <src>
    and <dest> are aligned on a four-byte boundary.
*/
#ifndef HAVE_memcpy_align4
#define HAVE_memcpy_align4
inline void memcpy_align4(void *dest, void *src, u32 count)
{
    if(count & 0x3)
    {
        switch(count)
        {
            case 3:     *((u8 *) dest++) = *((u8 *) src++);     /* Fall through */
            case 2:     *((u8 *) dest++) = *((u8 *) src++);     /* Fall through */
            case 1:     *((u8 *) dest++) = *((u8 *) src++);     /* Fall through */
        }
    }

    count >>= 2;

    while(count--)
        *((u32 *) dest++) = *((u32 *) src++);
};
#endif


/* Scatter/gather primitives */


/*
    mem_gather16v() - perform a "gather" operation to a 16-bit-wide volatile destination.  This
    primitive can be used to copy a block of data to a data port in a 16-bit peripheral.
*/
#ifndef HAVE_mem_gather16v
#define HAVE_mem_gather16v
inline void mem_gather16v(vu16 * restrict dest, ku16 * restrict src, u16 count)
{
    while(count--)
        *dest = *(src++);
};
#endif


/*
    mem_gather16v_zf() - perform a "gather" operation to a 16-bit-wide volatile destination,
    writing <count> 0x0000 words to the destination.  This is a specialised zero-filling version of
    mem_gather16v().
*/
#ifndef HAVE_mem_gather16v_zf
#define HAVE_mem_gather16v_zf
inline void mem_gather16v_zf(vu16 * restrict dest, u16 count)
{
    while(count--)
        *dest = 0;
};
#endif


/*
    mem_scatter16v() - perform a "scatter" operation from a 16-bit-wide volatile source.  This
    primitive can be used to copy a block of data from a data port in a 16-bit peripheral.
*/
#ifndef HAVE_mem_scatter16v
#define HAVE_mem_scatter16v
inline void mem_scatter16v(u16 * restrict dest, vu16 * restrict src, u16 count)
{
    while(count--)
        *(dest++) = *src;
};
#endif

#endif
