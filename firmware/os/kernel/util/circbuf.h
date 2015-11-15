#ifndef KERNEL_UTIL_CIRCBUF_H_INC
#define KERNEL_UTIL_CIRCBUF_H_INC
/*
    Circular buffer implementation

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, November 2015
*/

#include <include/defs.h>
#include <include/types.h>


/* Byte-oriented circular buffer structure */
typedef struct
{
    u8 *buf;
    u8 *buf_end;
    u8 *read_ptr;
    u8 *write_ptr;
} circbuf_t;


s32 circbuf_alloc(ku32 len, circbuf_t *cbuf);
s32 circbuf_free(circbuf_t *cbuf);
s32 circbuf_read(circbuf_t *cbuf, u8 *val);
s32 circbuf_write(circbuf_t *cbuf, u8 val);

#endif
