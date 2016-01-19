/*
	Console-related function definitions

	Part of ayumos


	(c) Stuart Wallace, January 2016.
*/

#include <kernel/console.h>


/*
    console_init() - specify the device to be used as the kernel console.  The device must support
    the putc() and getc() operations.
*/
void console_init(dev_t * console_dev)
{
    g_console_dev = console_dev;
}
