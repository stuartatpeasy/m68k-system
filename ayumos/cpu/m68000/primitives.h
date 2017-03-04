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


/* Scatter/gather primitives */


/*
    mem_gather16v() - "gather" <count> half-words from <src> into the volatile destination <dest>.
*/
#define HAVE_mem_gather16v
inline void mem_gather16v(ku16 * restrict src, vu16 * restrict dest, u16 count)
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
    asm volatile
    (
        "mem_gather16v_zf_%=:       bras mem_gather16v_zf_start_%=      \n"
        "mem_gather16v_zf_loop_%=:  clrw %0@+                           \n"
        "mem_gather16v_zf_start_%=: dbf %1, mem_gather16v_zf_loop_%=    \n"
        : "+a" (dest), "+d" (count)
        :
        : "cc", "memory"
    );
};


/*
    mem_scatter16v() - "scatter" <count> half-words from the volatile source <src> into destination
    <dest>.
*/
#define HAVE_mem_scatter16v
inline void mem_scatter16v(vu16 * restrict src, u16 * restrict dest, u16 count)
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
