/*
    Utility functions and macros implementing various memory buffer functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, November 2015
*/

#include <kernel/util/buffer.h>
#include <kernel/include/memory/kmalloc.h>
#include <klibc/include/string.h>


/*
    buffer_alloc() - allocate a fixed-length buffer object
*/
s32 buffer_alloc(ku32 len, buffer_t **buf)
{
    buffer_t *b = CHECKED_KMALLOC(sizeof(buffer_t));

    ks32 ret = buffer_init(len, b);
    if(ret != SUCCESS)
    {
        kfree(b);
        return ret;
    }

    *buf = b;
    return SUCCESS;
}


/*
    buffer_init() - given a buffer_t object, initialise its data area
*/
s32 buffer_init(ku32 len, buffer_t *buf)
{
    if(len)
    {
        void *data = CHECKED_KMALLOC(len);

        buf->data = data;
        buf->len = len;
    }
    else
    {
        buf->data = 0;
        buf->len = 0;
    }

    return SUCCESS;
}


/*
    buffer_deinit() - de-initialise a buffer, i.e. release the memory it points to and return it to
    an empty state.  This can be used to clean up buffers objects created on the stack.
*/
void buffer_deinit(buffer_t *buf)
{
    if(buf->len)
    {
        kfree(buf->data);
        buf->data = NULL;
        buf->len = 0;
    }
}


/*
    buffer_free() - release a buffer_t object
*/
void buffer_free(buffer_t *buf)
{
    buffer_deinit(buf);
    kfree(buf);
}


/*
    buffer_dup() - duplicate a buffer_t object
*/
s32 buffer_dup(const buffer_t * const buf, buffer_t **newbuf)
{
    s32 ret;
    buffer_t *b;

    ret = buffer_alloc(buf->len, &b);
    if(ret != SUCCESS)
        return ret;

    memcpy(b->data, buf->data, buf->len);
    *newbuf = b;

    return SUCCESS;
}


#ifdef WITH_LBUFFER
/*
    An "lbuffer" (name chosen more or less arbitrarily) is a fixed-length buffer allocated on the
    heap.  The struct describing the buffer (lbuffer_t) is contiguous with the buffer data area,
    meaning that only a single allocation is needed to create both the buffer object and the buffer
    itself.  Data in the buffer is accessed through the lbuffer_dptr() macro, which yields a void *
    pointer to the first byte of data in the buffer.  Example use:

        lbuffer_t *lbuf;

        if(lbuffer_alloc(1000, &lbuf) == SUCCESS)
        {
            void *start = lbuffer_dptr(lbuf);

            lbuffer_free(lbuf);
        }
*/

/*
    lbuffer_alloc() - allocate a lbuffer object
*/
s32 lbuffer_alloc(ku32 len, lbuffer_t **lbuf)
{
    lbuffer_t *lbuffer;

    if(!len)
        return -EINVAL;

    lbuffer = (lbuffer_t *) kmalloc(membersize(lbuffer_t, len) + len);
    if(lbuffer == NULL)
        return -ENOMEM;

    lbuffer->len = len;

    *lbuf = lbuffer;
    return SUCCESS;
}


/*
    lbuffer_free() - free a lbuffer object
*/
void lbuffer_free(lbuffer_t *lbuf)
{
    kfree(lbuf);
}


/*
    lbuffer_dup() - duplicate a lbuffer object
*/
s32 lbuffer_dup(const lbuffer_t * const lbuf, lbuffer_t **newlbuf)
{
    lbuffer_t *lb = (lbuffer_t *) kmalloc(membersize(lbuffer_t, len) + lbuf->len);
    if(lb == NULL)
        return -ENOMEM;

    lb->len = lbuf->len;
    memcpy(lbuffer_dptr(b), lbuffer_dptr(buf), lbuf->len);
    *newlbuf = lb;

    return SUCCESS;
}

#endif  /* WITH_LBUFFER */


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
        return -EAGAIN;         /* Buffer empty */

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
        return -EAGAIN;         /* Buffer full */

    *cbuf->write_ptr = val;
    cbuf->write_ptr = next;

    return SUCCESS;
}
