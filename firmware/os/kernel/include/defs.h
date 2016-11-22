#ifndef KERNEL_INCLUDE_DEFS_H_INC
#define KERNEL_INCLUDE_DEFS_H_INC
/*
	Miscellaneous definitions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, July 2012.
*/

#include <kernel/include/types.h>


/* FIXME - target-specific */
#if defined(TARGET_MC68008)
#define DATA_BUS_WIDTH  (8)
#elif defined(TARGET_MC68000) || defined(TARGET_MC68010)
#define DATA_BUS_WIDTH  (16)
#else
#define DATA_BUS_WIDTH  (32)
#endif

/* API-like functions try to return zero on success and nonzero on failure */
#define SUCCESS			(0)

/* Directory separator character used in file paths */
#define DIR_SEPARATOR	('/')

/* Block device block size */
#define BLOCK_SIZE		(512)
#define LOG_BLOCK_SIZE	(9)

/* Maximum length of a name (i.e. a component in a file system path) */
#define NAME_MAX_LEN	(255)

/* "Tick rate" - number of timer interrupts per second */
#define TICK_RATE       (64)

/* Length of kernel stack */
#define KERNEL_STACK_LEN        (8 * 1024)          /* Kernel stack is 8KB */

/* "Stringification" macros - see https://gcc.gnu.org/onlinedocs/cpp/Stringification.html */
#define STRINGIFY(s)    STRINGIFY_(s)
#define STRINGIFY_(s)   #s

/* CHECKED_CALL evaluates expr, which is normally a function call.  If expr (the call's return
   value) is anything other than SUCCESS, the value is returned. */
#define CHECKED_CALL(expr)      \
{                               \
    ks32 ret = (expr);          \
    if(ret != SUCCESS)          \
        return ret;             \
}

/* Obtain the number of elements in a static array. */
#define ARRAY_COUNT(a)  (sizeof(a) / sizeof((a)[0]))

/* Iterate over a static array. */
#define FOR_EACH(p, a) \
    for(p = (a); p < &a[ARRAY_COUNT(a)]; ++p)


/* Return the offset of member m in struct st.  Not strictly-valid C. */
#define offsetof(st, m) ((s32) (&((st *) 0)->m))

/* Given ptr, pointing to a member of struct "type", return a ptr to the struct */
#define containerof(ptr, type, member)                  \
({                                                      \
    const typeof(((type *) 0)->member) *mptr = (ptr);   \
    (type *) ((char *) mptr - offsetof(type, member));  \
})

/* Obtain the size of a struct member */
#define membersize(type, member)        sizeof(((type *) 0)->member)

/* Return the minimum of two values; accesses each value exactly once. */
#define MIN(a, b)           \
({                          \
    typeof(a) _a = (a);     \
    typeof(b) _b = (b);     \
    _a < _b ? _a : _b;      \
})

/* Return the maximum of two values; accesses each value exactly once. */
#define MAX(a, b)           \
({                          \
    typeof(a) _a = (a);     \
    typeof(b) _b = (b);     \
    _a > _b ? _a : _b;      \
})

/* Return a val with bit x set (i.e. compute 2^x). */
#define BIT(x)          (1 << (x))

/* Explicitly (and portably) "declare" an argument unused, silencing an "unused arg" warning. */
#define UNUSED(x)       (void) (x)

/*
	CEIL_LOG2() - use the preprocessor to compute ceil(log2(x)), where 0 < x < 2^32.  x should be
	something the preprocessor knows to be constant, e.g. sizeof(foo).

	Note: yields 0 for x < 1.
*/
#define CEIL_LOG2(x) \
	  (((x) > (1 << 30)) ? 31 \
	: (((x) > (1 << 29)) ? 30 \
	: (((x) > (1 << 28)) ? 29 \
	: (((x) > (1 << 27)) ? 28 \
	: (((x) > (1 << 26)) ? 27 \
	: (((x) > (1 << 25)) ? 26 \
	: (((x) > (1 << 24)) ? 25 \
	: (((x) > (1 << 23)) ? 24 \
	: (((x) > (1 << 22)) ? 23 \
	: (((x) > (1 << 21)) ? 22 \
	: (((x) > (1 << 20)) ? 21 \
	: (((x) > (1 << 19)) ? 20 \
	: (((x) > (1 << 18)) ? 19 \
	: (((x) > (1 << 17)) ? 18 \
	: (((x) > (1 << 16)) ? 17 \
	: (((x) > (1 << 15)) ? 16 \
	: (((x) > (1 << 14)) ? 15 \
	: (((x) > (1 << 13)) ? 14 \
	: (((x) > (1 << 12)) ? 13 \
	: (((x) > (1 << 11)) ? 12 \
	: (((x) > (1 << 10)) ? 11 \
	: (((x) > (1 <<  9)) ? 10 \
	: (((x) > (1 <<  8)) ?  9 \
	: (((x) > (1 <<  7)) ?  8 \
	: (((x) > (1 <<  6)) ?  7 \
	: (((x) > (1 <<  5)) ?  6 \
	: (((x) > (1 <<  4)) ?  5 \
	: (((x) > (1 <<  3)) ?  4 \
	: (((x) > (1 <<  2)) ?  3 \
	: (((x) > (1 <<  1)) ?  2 \
	: (((x) > (1 <<  0)) ?  1 \
	: 0)))))))))))))))))))))))))))))))


extern u8 _sdata,       /* .data section start      */
          _edata,       /* .data section end        */
          _sbss,        /* .bss  section start      */
          _ebss,        /* .bss  section end        */
          _stext,       /* .text section start      */
          _etext,       /* .text section end        */
          _ssym;        /* Start of kernel symbols  */

extern u32 g_ram_top;   /* Address of first byte past the end of RAM.  Set by ram_detect() */

#ifndef NULL
#define NULL ((void *) 0)
#endif

#endif
