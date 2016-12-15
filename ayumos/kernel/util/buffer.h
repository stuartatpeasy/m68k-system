#ifndef KERNEL_UTIL_BUFFER_H_INC
#define KERNEL_UTIL_BUFFER_H_INC
/*
    Utility functions and macros implementing various memory buffer functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, November 2015
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>


#define CIRCBUF(type)   \
    struct              \
    {                   \
        type data[256]; \
        u8 rd;          \
        u8 wr;          \
    }

#define CIRCBUF_INIT(buf)           { buf.rd = 0; buf.wr = 0; }

#define CIRCBUF_WRITE(buf, val)     (buf.data[buf.wr++] = (val))
#define CIRCBUF_READ(buf)           (buf.data[buf.rd++])

#define CIRCBUF_IS_EMPTY(buf)       (buf.rd == buf.wr)
#define CIRCBUF_IS_FULL(buf)        (buf.rd == (buf.wr + 1))

#define CIRCBUF_COUNT(buf)          (buf.wr - buf.rd)


/* buffer: fixed-length buffer */
typedef struct buffer
{
    u32     len;
    void    *data;
} buffer_t;


s32 buffer_alloc(ku32 len, buffer_t **buf);
s32 buffer_init(ku32 len, buffer_t *buf);
void buffer_deinit(buffer_t *buf);
void buffer_free(buffer_t *buf);
s32 buffer_dup(const buffer_t * const buf, buffer_t **newbuf);


#ifdef WITH_LBUFFER

/* lbuffer: fixed-length buffer allocated as a single region of heap. See buffer.c for info */
typedef struct lbuffer
{
    u32     len;
    u8      data_start;
} lbuffer_t;

/* Obtain a pointer (of type void *) to the data area in a lbuffer object */
#define lbuffer_dptr(lbuf)        ((void *) &((lbuf)->data_start))

s32 lbuffer_alloc(ku32 len, buffer_t **lbuf);
void lbuffer_free(lbuffer_t *buf);
s32 lbuffer_dup(const lbuffer_t * const lbuf, lbuffer_t **newlbuf);

#endif  /* WITH_LBUFFER */


/* Byte-oriented circular buffer structure */
typedef struct circbuf
{
    u8 *    buf;
    u8 *    buf_end;
    u8 *    read_ptr;
    u8 *    write_ptr;
} circbuf_t;


s32 circbuf_alloc(ku32 len, circbuf_t *cbuf);
s32 circbuf_free(circbuf_t *cbuf);
s32 circbuf_read(circbuf_t *cbuf, u8 *val);
s32 circbuf_write(circbuf_t *cbuf, u8 val);

#endif
