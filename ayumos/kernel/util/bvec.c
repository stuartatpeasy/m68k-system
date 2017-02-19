/*
    bvec.c - block-allocating dynamic array (vector) implementation

    Part of the as-yet-unnamed MC68010 operating system

    (c) Stuart Wallace <stuartw@atom.net>, 2011-15

    This module implements an extensible, block-allocated variable-length array.  The constructor
    function, bvec_init(), allocates enough space for block_size elements of size element_size.
    This is a "block".  Once block_size elements have been added to the vector, i.e. the block is
    full, another block is allocated.  Elements are added using the bvec_grow() method and
    accessed using bvec_get() (which is bounds-checked) or directly via vector->elements[n].  The
    size of the vector (excluding "free" elements) can be obtained using bvec_size().  A simple
    iterator function, bvec_iterate(), allows a user function to be called for each element in the
    vector.  The vector must be deallocated by a call to the bvec_destroy() function.  Deletion
    of elements is not implemented.

    This container is designed to make allocation of elements more efficient by minimising the
    number of calls to *alloc() functions.  Also, it preserves pointers to individual elements:
    after resizing, any pointers to existing elements remain valid for the lifetime of the vector.
*/


#include <kernel/util/bvec.h>


u32 bvec_init(u32 block_size, ku32 element_size, bvec_t *bv)
{
    bvec_t bvec;
    u32 u;

    /* ensure that the requested block and elements sizes are nonzero */
    if(!block_size || !element_size)
        return -EINVAL;

    bvec = (bvec_t) kmalloc(sizeof(struct bvec));
    if(!bvec)
        return -ENOMEM;

    /* the vector will initially contain one block's worth of elements */
    bvec->elements = kmalloc(block_size * sizeof(void *));
    if(!bvec->elements)
    {
        kfree(bvec);
        return -ENOMEM;
    }

    /* allocate initial elements */
    bvec->elements[0] = kmalloc(block_size * element_size);
    if(!bvec->elements[0])
    {
        kfree(bvec->elements);
        kfree(bvec);
        return -ENOMEM;
    }

    /* fill in element pointers */
    for(u = 1; u < block_size; ++u)
        bvec->elements[u] = bvec->elements[u - 1] + element_size;

    bvec->block_size = block_size;
    bvec->element_size = element_size;
    bvec->nelements = 0;
    bvec->free_elements = block_size;

    *bv = bvec;

    return SUCCESS;
}


u32 bvec_destroy(bvec_t *v)
{
    u32 i;
    for(i = 0; i < (*v)->nelements; i += (*v)->block_size)
        kfree((*v)->elements[i]);

    kfree((*v)->elements);
    kfree(*v);

    return SUCCESS;
}


void *bvec_grow(bvec_t v)
{
    if(!v->free_elements)
    {
        /* No free elements.  Allocate a new block. */
        void **element_ptr, *new_elements;
        u32 u;

        /* First extend the element pointer array */
        element_ptr = (void **) kmalloc((v->nelements + v->block_size) * sizeof(void *));
        if(!element_ptr)
        {
            return NULL;    /* out of memory */
        }

        new_elements = kmalloc(v->block_size * v->element_size);
        if(!new_elements)
        {
            kfree(element_ptr);
            return NULL;    /* out of memory */
        }

        /* Copy the existing element pointers */
        memcpy(element_ptr, v->elements, v->nelements * sizeof(void *));

        /* Copy in the new element pointers */
        for(u = 0; u < v->block_size; ++u)
            element_ptr[v->nelements + u] = new_elements + (u * v->element_size);

        kfree(v->elements);

        v->elements = element_ptr;
        v->free_elements = v->block_size - 1;
    }
    else
    {
        /* there is at least one free element available */
        --v->free_elements;
    }

    return v->elements[v->nelements++];
}


u32 bvec_size(bvec_t v)
{
    return v->nelements;
}


void *bvec_get(bvec_t v, ku32 item)
{
    return (item < v->nelements) ? v->elements[item] : NULL;
}


u32 bvec_iterate(bvec_t v, bvec_unary_predicate f, u32 x)
{
    void **p = v->elements;
    u32 n = v->nelements;

    for(; n--; ++p)
    {
        ku32 ret = f(*p, x);
        if(ret)
            return ret;
    }

    return 0;
}

