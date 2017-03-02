#ifndef CPU_M68000_PRIMITIVES_H_INC
#define CPU_M68000_PRIMITIVES_H_INC
/*
    m68000/m68010-specific versions of the memory-primitive functions.

    Part of ayumos

    (c) Stuart Wallace <stuartw@atom.net>, March 2017.
*/


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
        : "=a" (src)
        : "a" (dest), "d" (count)
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
        :
        : "a" (dest), "d" (count)
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
