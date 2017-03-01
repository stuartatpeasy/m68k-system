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


/*
    mem_gather16v() - perform a "gather" operation to a 16-bit-wide volatile destination.  This
    primitive can be used to copy a block of data to a data port in a 16-bit peripheral.
*/
#ifndef HAVE_mem_gather16v
inline void mem_gather16v(ku16 * restrict src, vu16 * restrict const dest, u16 count)
{
    while(count--)
        *dest = *(src++);
};
#define HAVE_mem_gather16v
#endif


/*
    mem_gather16v_zf() - perform a "gather" operation to a 16-bit-wide volatile destination,
    writing <count> 0x0000 words to the destination.  This is a specialised zero-filling version of
    mem_gather16v().
*/
#ifndef HAVE_mem_gather16v_zf
inline void mem_gather16v_zf(vu16 * restrict const dest, u16 count)
{
    while(count--)
        *dest = 0;
}
#define HAVE_mem_gather16v_zf
#endif

/*
    mem_scatter16v() - perform a "scatter" operation from a 16-bit-wide volatile source.  This
    primitive can be used to copy a block of data from a data port in a 16-bit peripheral.
*/
#ifndef HAVE_mem_scatter16v
inline void mem_scatter16v(vu16 * restrict const src, u16 * restrict dest, u16 count)
{
    while(count--)
        *(dest++) = *src;
};
#define HAVE_mem_scatter16v
#endif

#endif
