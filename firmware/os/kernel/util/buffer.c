/*
    Utility functions and macros implementing various memory buffer functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, November 2015
*/

#include <kernel/util/buffer.h>
#include <kernel/memory/kmalloc.h>
#include <klibc/string.h>


/*
    buffer_alloc() - allocate a buffer object
*/
s32 buffer_alloc(ku32 len, buffer_t **buf)
{
    buffer_t *buffer;

    if(!len)
        return EINVAL;

    buffer = (buffer_t *) kmalloc(membersize(buffer_t, len) + len);
    if(buffer == NULL)
        return ENOMEM;

    buffer->len = len;

    *buf = buffer;
    return SUCCESS;
}


/*
    buffer_free() - free a buffer object
*/
void buffer_free(buffer_t *buf)
{
    kfree(buf);
}


/*
    buffer_dup() - duplicate a buffer object
*/
s32 buffer_dup(const buffer_t * const buf, buffer_t **newbuf)
{
    buffer_t *b = (buffer_t *) kmalloc(membersize(buffer_t, len) + buf->len);
    if(b == NULL)
        return ENOMEM;

    b->len = buf->len;
    memcpy(buffer_dptr(b), buffer_dptr(buf), buf->len);
    *newbuf = b;

    return SUCCESS;
}


/*
    circbuf_alloc() - initialise a circular buffer of length len.
*/
s32 circbuf_alloc(ku32 len, circbuf_t *cbuf)
{
    cbuf->buf = CHECKED_KMALLOC(len);

    cbuf->buf_end = cbuf->buf + len;
    cbuf->read_ptr = cbuf->buf;
    cbuf->write_ptr = cbuf->buf;

    return SUCCESS;
}


/*
    circbuf_free() - release memory associated with a circular buffer.
*/
s32 circbuf_free(circbuf_t *cbuf)
{
    if(cbuf->buf != NULL)
        kfree(cbuf->buf);

    return SUCCESS;
}


/*
    circbuf_read() - attempt to read a byte from the buffer.  Fail if the buffer is empty.
*/
s32 circbuf_read(circbuf_t *cbuf, u8 *val)
{
    if(cbuf->read_ptr == cbuf->write_ptr)
        return EAGAIN;          /* Buffer empty */

    *val = *cbuf->read_ptr++;
    if(cbuf->read_ptr == cbuf->buf_end)
        cbuf->read_ptr = cbuf->buf;

    return SUCCESS;
}


/*
    circbuf_write() - attempt to write a byte to the buffer.  Fail if the buffer is empty.
*/
s32 circbuf_write(circbuf_t *cbuf, u8 val)
{
    u8 *next = cbuf->write_ptr + 1;
    if(next == cbuf->buf_end)
        next = cbuf->buf;

    if(next == cbuf->read_ptr)
        return EAGAIN;          /* Buffer full */

    *cbuf->write_ptr = val;
    cbuf->write_ptr = next;

    return SUCCESS;
}
