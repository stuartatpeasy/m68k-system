#ifndef INCLUDE_DEFS_H_INC
#define INCLUDE_DEFS_H_INC
/*
	Miscellaneous definitions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, July 2012.
*/

#include <include/types.h>

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
#define TICK_RATE       (100)

/* Per-process stack size */
#define PROC_STACK_SIZE (16 * 1024)

/* CHECKED_CALL evaluates expr, which is normally a function call.  If expr (the call's return
   value) is anything other than SUCCESS, the value is returned. */
#define CHECKED_CALL(expr)      \
{                               \
    ku32 ret = (expr);          \
    if(ret != SUCCESS)          \
        return ret;             \
}

/* Obtain the number of elements in a static array. */
#define ARRAY_COUNT(a)  (sizeof(a) / sizeof((a)[0]))

/* Iterate over a static array. */
#define FOR_EACH(p, a) \
    for(p = (a); p < &a[ARRAY_COUNT(a)]; ++p)


/* Return the offset of member m in struct st.  Not strictly-valid C. */
#define offsetof(st, m) ((size_t) (&((st *) 0)->m))

/* Given ptr, pointing to a member of struct "type", return a ptr to the struct */
#define containerof(ptr, type, member)                  \
({                                                      \
    const typeof(((type *) 0)->member) *mptr = (ptr);   \
    (type *) ((char *) mptr - offsetof(type, member));  \
})

/* Return a val with bit x set (i.e. compute 2^x). */
#define BIT(x)          (1 << (x))

/* Explicitly (and portably) "declare" an argument unused, silencing an "unused arg" warning. */
#define UNUSED(x)       (void) (x)


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
