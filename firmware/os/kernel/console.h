#ifndef KERNEL_CONSOLE_H_INC
#define KERNEL_CONSOLE_H_INC
/*
	Console-related function definitions

	Part of ayumos


	(c) Stuart Wallace, January 2016.
*/

#include <kernel/device/device.h>
#include <kernel/include/defs.h>
#include <kernel/include/types.h>


dev_t *g_console_dev;

void console_init(dev_t * const console_dev);


/*
    console_putc() - write a character to the kernel console device.  May block.
*/
inline s32 console_putc(const char c)
{
    return g_console_dev->putc(g_console_dev, c);
}


/*
    console_getc() - read a character from the kernel console device.  May block.
*/
inline s16 console_getc()
{
    return g_console_dev->getc(g_console_dev);
}

#endif
