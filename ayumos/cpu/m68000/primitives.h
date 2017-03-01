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
inline void mem_gather16v(ku16 * restrict src, vu16 * restrict const dest, u16 count)
{
    asm volatile
    (
        "L_%=:                          \n"
        "        movew %0@, %1@+        \n"
        "        dbf %2, L_%=           \n"
        :
        : "a" (src), "a" (dest), "d" (count)
        : "cc"
    );
};


/*
    mem_gather16v_zf() - "gather" <count> half-word zeroes into the volatile destination <dest>.
*/
#define HAVE_mem_gather16v_zf
inline void mem_gather16v_zf(vu16 * restrict const dest, u16 count)
{
    asm volatile
    (
        "L_%=:                          \n"
        "        clrw %0@+              \n"
        "        dbf %1, L_%=           \n"
        :
        : "a" (dest), "d" (count)
        : "cc"
    );
};


/*
    mem_scatter16v() - "scatter" <count> half-words from the volatile source <src> into destination
    <dest>.
*/
#define HAVE_mem_scatter16v
inline void mem_scatter16v(vu16 * restrict const src, u16 * restrict dest, u16 count)
{
    asm volatile
    (
        "L_%=:                          \n"
        "        movew %1@, %0@+        \n"
        "        dbf %2, L_%=           \n"
        :
        : "a" (src), "a" (dest), "d" (count)
        : "cc"
    );
};

#endif
