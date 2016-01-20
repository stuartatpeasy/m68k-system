/*
	In-memory "console" device driver

	Part of ayumos


    This device is used as a temporary console early in the system boot process.  It implements a
    putc() operation to capture console output generated before the real system console is ready
    (i.e. before interrupts are enabled).  It exposes a getc() operation which allows text already
    sent to the console to be read back.  Note that getc() doesn't read user input - it just returns
    the data previously written using putc().

	(c) Stuart Wallace, January 2016.
*/

#include <kernel/device/memconsole.h>
#include <kernel/memory/kmalloc.h>


/*
    memconsole_init() - initalise an in-memory "console" device.
*/
s32 memconsole_init(dev_t *dev)
{
    memconsole_state_t *state = (memconsole_state_t *) kmalloc(sizeof(memconsole_state_t));

    dev->data = 0;

    if(!state)
        return ENOMEM;

    state->buffer = (char *) kmalloc(MEMCONSOLE_DEFAULT_SIZE);

    state->end = MEMCONSOLE_DEFAULT_SIZE - 1;
    state->rd_ptr = 0;
    state->wr_ptr = 0;

    dev->data = state;
    dev->block_size = 1;
    dev->len = MEMCONSOLE_DEFAULT_SIZE;

    dev->getc = memconsole_getc;
    dev->putc = memconsole_putc;
    dev->shut_down = memconsole_shut_down;

    return SUCCESS;
}


/*
    memconsole_getc() - read a character from the in-memory "console" buffer.  If no more characters
    can be read from the buffer, return -EAGAIN.
*/
s16 memconsole_getc(dev_t *dev)
{
    memconsole_state_t * const state = (memconsole_state_t *) dev->data;

    if(state && (state->rd_ptr < state->wr_ptr))
        return state->buffer[state->rd_ptr++];

    return -EAGAIN;
}


/*
    memconsole_putc() - write a character to the in-memory "console" buffer.  If the buffer is full,
    drop the character and return ENOSPC.
*/
s32 memconsole_putc(dev_t *dev, const char c)
{
    memconsole_state_t * const state = (memconsole_state_t *) dev->data;

    if(state && (state->wr_ptr < state->end))
    {
        state->buffer[state->wr_ptr++] = c;
        return SUCCESS;
    }

    return ENOSPC;
}


/*
    memconsole_shut_down() - free the memory associated with an in-memory "console" buffer object.
*/
s32 memconsole_shut_down(dev_t *dev)
{
    if(dev)
    {
        memconsole_state_t * const state = (memconsole_state_t *) dev->data;
        if(state->buffer)
            kfree(state->buffer);

        state->buffer = 0;
        kfree(state);
    }

    return SUCCESS;
}
