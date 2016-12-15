#ifndef KERNEL_INCLUDE_DEVICE_MEMCONSOLE_H_INC
#define KERNEL_INCLUDE_DEVICE_MEMCONSOLE_H_INC
/*
	In-memory "console" device driver

	Part of ayumos


	(c) Stuart Wallace, January 2016.
*/

#include <kernel/include/device/device.h>
#include <kernel/include/defs.h>
#include <kernel/include/types.h>


/* Default size of the memory buffer associated with an in-memory console */
#define MEMCONSOLE_DEFAULT_SIZE     (16384)

typedef struct memconsole_state
{
    char *buffer;
    s32 end;
    s32 rd_ptr;
    s32 wr_ptr;
} memconsole_state_t;


s32 memconsole_init(dev_t *dev);
s16 memconsole_getc(dev_t *dev);
s32 memconsole_putc(dev_t *dev, const char c);
s32 memconsole_shut_down(dev_t *dev);

#endif
