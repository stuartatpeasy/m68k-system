#ifndef CPU_M68000_PRIMITIVES_H_INC
#define CPU_M68000_PRIMITIVES_H_INC
/*
    m68000/m68010-specific versions of the memory-primitive functions.

    Part of ayumos

    (c) Stuart Wallace <stuartw@atom.net>, March 2017.
*/


/* Byte-/word-swapping primitives */


/*
    bswap_16() - swap the endianness of a 16-bit half-word.
*/
#define HAVE_bswap_16
inline u16 bswap_16(u16 x)
{
    register u16 x_ = x;
    asm volatile
    (
        "bswap_16_%=:               rol.w #8, %w0           \n"
        : "+d" (x_)
        :
        : "cc"
    );

    return x_;
};


/*
    bswap_32() - swap the endianness of a 32-bit word.
*/
#define HAVE_bswap_32
inline u32 bswap_32(u32 x)
{
    register u32 x_ = x;
    asm volatile
    (
        "bswap_32_%=:               rol.w #8, %0            \n"
        "                           swap %0                 \n"
        "                           rol.w #8, %0            \n"
        : "+d" (x_)
        :
        : "cc"
    );

    return x_;
};


/*
    wswap_32() - swap the 16-bit half-words in a 32-bit word.
*/
#define HAVE_wswap_32
inline u32 wswap_32(u32 x)
{
    register u32 x_ = x;
    asm volatile
    (
        "wswap_32_%=:               swap %0                 \n"
        : "+d" (x_)
        :
        : "cc"
    );

    return x_;
};


/* Memory copying / zeroing primitives */

/*
    memcpy_align4() - copy <count> bytes from <src> to <dest>, with the assumption that both <src>
    and <dest> are aligned on a four-byte boundary.
*/
#define HAVE_memcpy_align4
inline void memcpy_align4(void *dest, void *src, u32 count)
{
    u16 iter, count_;
    u8 trail = count, *src_, *dest_;

    trail = count & 0x3;
    count >>= 2;
    iter = count >> 16;
    count_ = (u16) count;

    do
    {
        asm volatile
        (
            "memcpy_align4_%=:          bras memcpy_align4_start_%=         \n"
            "memcpy_align4_loop_%=:     movel %0@+, %1@+                    \n"
            "memcpy_align4_start_%=:    dbf %2, memcpy_align4_loop_%=       \n"
            : "+a" (src), "+a" (dest), "+d" (count_)
            :
            : "cc", "memory"
        );
    } while(iter--);

    src_ = (u8 *) src;
    dest_ = (u8 *) dest;

    switch(trail)
    {
        case 3:     *dest_++ = *src_++;     /* Fall through */
        case 2:     *dest_++ = *src_++;     /* Fall through */
        case 1:     *dest_++ = *src_++;     /* Fall through */
        default:
            ;       /* Do nothing */
    }
};


/* Scatter/gather primitives */


/*
    mem_gather16v() - "gather" <count> half-words from <src> into the volatile destination <dest>.
*/
#define HAVE_mem_gather16v
inline void mem_gather16v(vu16 * restrict dest, ku16 * restrict src, u16 count)
{
    asm volatile
    (
        "mem_gather16v_%=:          bras mem_gather16v_start_%=         \n"
        "mem_gather16v_loop_%=:     movew %0@, %1@+                     \n"
        "mem_gather16v_start_%=:    dbf %2, mem_gather16v_loop_%=       \n"
        : "+a" (src), "+a" (dest), "+d" (count)
        :
        : "cc", "memory"
    );
};


/*
    mem_gather16v_zf() - "gather" <count> half-word zeroes into the volatile destination <dest>.
*/
#define HAVE_mem_gather16v_zf
inline void mem_gather16v_zf(vu16 * restrict dest, u16 count)
{
#if defined(TARGET_MC68000) || defined(TARGET_MC68008)
    /*
        The MC68000/MC68008 has a microcode bug causing the "clr" instruction to read the location
        to be cleared before zeroing it.  For the MC68000, we therefore clear a register and write
        it to the destination location instead of using clr directly on the destination.
    */
    asm volatile
    (
        "mem_gather16v_zf_%=:       clrw %%d0                           \n"
        "                           bras mem_gather16v_zf_start_%=      \n"
        "mem_gather16v_zf_loop_%=:  movew %%d0, %0@+                    \n"
        "mem_gather16v_zf_start_%=: dbf %1, mem_gather16v_zf_loop_%=    \n"
        : "+a" (dest), "+d" (count)
        :
        : "cc", "memory", "d0"
    );
#else
    asm volatile
    (
        "mem_gather16v_zf_%=:       bras mem_gather16v_zf_start_%=      \n"
        "mem_gather16v_zf_loop_%=:  clrw %0@+                           \n"
        "mem_gather16v_zf_start_%=: dbf %1, mem_gather16v_zf_loop_%=    \n"
        : "+a" (dest), "+d" (count)
        :
        : "cc", "memory"
    );
#endif
};


/*
    mem_scatter16v() - "scatter" <count> half-words from the volatile source <src> into destination
    <dest>.
*/
#define HAVE_mem_scatter16v
inline void mem_scatter16v(u16 * restrict dest, vu16 * restrict src, u16 count)
{
    asm volatile
    (
        "mem_scatter16v_%=:         bras mem_scatter16v_start_%=        \n"
        "mem_scatter16v_loop_%=:    movew %0@, %1@+                     \n"
        "mem_scatter16v_start_%=:   dbf %2, mem_scatter16v_loop_%=      \n"
        : "+a" (src), "+a" (dest), "+d" (count)
        :
        : "cc", "memory"
    );
};

#endif
