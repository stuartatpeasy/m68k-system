#ifndef KERNEL_INCLUDE_MEMORY_KMALLOC_H_INC
#define KERNEL_INCLUDE_MEMORY_KMALLOC_H_INC
/*
	kmalloc.h - kernel memory allocation/deallocation functions

	Part of the as-yet-unnamed MC68010 operating system

	(c) Stuart Wallace <stuartw@atom.net>, July 2012
*/

#include <kernel/include/types.h>
#include <klibc/include/errno.h>


/* If no allocator was specified in build options, use the heap allocator */
#if (!defined(KMALLOC_HEAP) && !defined(KMALLOC_BUDDY))
#define KMALLOC_HEAP
#endif

#if defined(KMALLOC_HEAP)
/*
	Use heap allocator
*/

#include <kernel/include/memory/heap.h>

#define ALLOCATOR_FN(name) heap_##name
typedef heap_ctx mem_ctx;

#elif defined(KMALLOC_BUDDY)
/*
	Use buddy allocator
*/

#ifndef BUDDY_MIN_ALLOC_UNIT
#define BUDDY_MIN_ALLOC_UNIT	(512)		/* bytes */
#endif

#include <kernel/memory/buddy.h>

#define ALLOCATOR_FN(name) buddy_##name
typedef buddy_ctx mem_ctx;

#else
#error "No memory allocator specified (try -DKMALLOC_HEAP or -DKMALLOC_BUDDY)"
#endif

/* mem_ctx is a struct defined by whichever allocator is selected */
mem_ctx g_kheap;    /* kernel heap */
mem_ctx g_uheap;    /* user heap (shared by all userland processes) */

void kmeminit(void * const start, void * const end);
void umeminit(void * const start, void * const end);

/*
    Allocator functions
*/

void *kmalloc(u32 size);
void *kcalloc(ku32 nmemb, ku32 size);
void *krealloc(void *ptr, u32 size);
void kfree(void *ptr);
u32 kfreemem();
u32 kusedmem();

void *umalloc(u32 size);
void *ucalloc(ku32 nmemb, ku32 size);
void *urealloc(void *ptr, u32 size);
void ufree(void *ptr);
u32 ufreemem();
u32 uusedmem();

/*
    Helper macros to implement the common case of calling *malloc(), checking for ret == NULL,
    and returning ENOMEM if so.
*/
#define CHECKED_KMALLOC(size)           \
({                                      \
    void *_ptr = kmalloc(size);         \
    if(_ptr == NULL)                    \
        return ENOMEM;                  \
    _ptr;                               \
})

#define CHECKED_KCALLOC(nmemb, size)    \
({                                      \
    void *_ptr = kcalloc(nmemb, size);  \
    if(_ptr == NULL)                    \
        return ENOMEM;                  \
    _ptr;                               \
})

#define CHECKED_UMALLOC(size)           \
({                                      \
    void *_ptr = umalloc(size);         \
    if(_ptr) == NULL)                   \
        return ENOMEM;                  \
    _ptr;                               \
})

#define CHECKED_UCALLOC(nmemb, size)    \
({                                      \
    void *_ptr = ucalloc(nmemb, size);  \
    if(_ptr) == NULL)                   \
        return ENOMEM;                  \
    _ptr;                               \
})


#endif
