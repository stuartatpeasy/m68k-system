/*
    Circular buffer implementation

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, November 2015
*/

#include <kernel/util/circbuf.h>
#include <kernel/memory/kmalloc.h>


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
