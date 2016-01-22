/*
	Console-related function definitions

	Part of ayumos


	(c) Stuart Wallace, January 2016.
*/

#include <kernel/console.h>
#include <kernel/cpu.h>
#include <kernel/device/memconsole.h>


dev_t *g_console_dev = NULL;
dev_t *g_early_boot_console = NULL;


/*
    console_init() - specify the device to be used as the kernel console.  The device must support
    the putc() and getc() operations.  Returns a pointer to the previous console device, if any.
*/
dev_t *console_set_device(dev_t * console_dev)
{
    dev_t * const old_dev = g_console_dev;

    g_console_dev = console_dev;

    return old_dev;
}


/*
    early_boot_console_init() - initialise an in-memory "console" to be used until the real console
    becomes available.
*/
s32 early_boot_console_init()
{
    s32 ret;

    ret = dev_create(DEV_TYPE_CHARACTER, DEV_SUBTYPE_NONE, "con", IRQL_NONE, NULL,
                     &g_early_boot_console, "Early-boot console", NULL, memconsole_init);
    if(ret != SUCCESS)
        return ret;

    console_set_device(g_early_boot_console);

    return SUCCESS;
}


/*
    early_boot_console_close() - copy the contents of the early-boot console to the real console
    and free the early-boot console device.
*/
void early_boot_console_close()
{
    char c;

    while((c = g_early_boot_console->getc(g_early_boot_console)) >= 0)
        console_putc(c);

    dev_destroy(g_early_boot_console);
}
