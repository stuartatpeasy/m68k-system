/*
	device.c: device and device driver management

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 9th Febrary 2012.
*/

#include "errno.h"
#include "stdio.h"        /* FIXME - remove */
#include "string.h"
#include "strings.h"

#include "kutil/kutil.h"
#include "device/device.h"

/* Characters used to identify "sub-devices", e.g. partitions of devices.  The first sub-device
 * of device xxx will be xxx1, the second xxx2, ..., the 61st xxxZ */
const char * const g_device_sub_names = "123456789abcdefghijklmnopqrstuv"
                                        "wxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

device_t *g_devices[MAX_DEVICES];

/*
    #include driver registration function declarations here
*/
#include "device/block/ata/ata.h"
#include "device/block/partition/partition.h"

device_driver_t *g_drivers[] =
{
    &g_ata_driver,
    &g_partition_driver
};


u32 driver_init()
{
	const u32 num_drivers = sizeof(g_drivers) / sizeof(g_drivers[0]);
	u32 x;

	/* TODO: is this necessary?  Presumably not if g_devices is in bss seg */
	bzero(g_devices, sizeof(g_devices));

	for(x = 0; x < num_drivers; ++x)
	{
	    device_driver_t * const drv = g_drivers[x];

		if(drv->init() != DRIVER_OK)	/* init() will create devices */
		{
			printf("%s: driver initialisation failed\n", drv->name);

			/* Ensure that none of this driver's methods are callable */
		    drv->read = driver_method_not_implemented;
		    drv->write = driver_method_not_implemented;
		    drv->shut_down = driver_method_not_implemented;
		    drv->control = driver_method_not_implemented;
		}
	}

	return SUCCESS;
}


/*
    Generic function which can be used by drivers to indicate that a particular operation isn't
    supported.  Also, if a driver fails to initialise, all of its operation funcptrs will be
    replaced with a call to this fn.
*/
u32 driver_method_not_implemented()
{
    return DRIVER_NOT_IMPLEMENTED;
}


void driver_shut_down()
{
	/* TODO: ensure all devices are stopped/flushed/etc */
	u32 x;

	const u32 num_drivers = sizeof(g_drivers) / sizeof(g_drivers[0]);
	for(x = 0; x < num_drivers; ++x)
	{
		if(g_drivers[x]->shut_down() != DRIVER_OK)
		{
			/* TODO: error */
			printf("Failed to shut down '%s' driver\n", g_drivers[x]->name);
		}
	}

	for(x = 0; x < (sizeof(g_devices) / sizeof(g_devices[0])); ++x)
    {
        if(g_devices[x])
            kfree(g_devices[x]);
    }
}


device_t *get_device_by_name(const char * const name)
{
	int i;
	for(i = 0; i < (sizeof(g_devices) / sizeof(g_devices[0])); ++i)
	{
	    if(g_devices[i] != NULL)
        {
            if(!strcmp(g_devices[i]->name, name))
                return g_devices[i];
        }
	}

	return NULL;	/* No such device */
}


/* Create (i.e. register) a new device.  This function is called by each driver's init() function
 * during device enumeration. */
driver_ret create_device(const device_type_t type, const device_class_t class,
                         device_driver_t * const driver, const char *name, void * const data)
{
    u32 u;
    device_t * dev;

    /* Find a vacant device ID */
    for(u = 0; u < MAX_DEVICES; ++u)
    {
        if(g_devices[u] == NULL)
            break;
    }

    if(u == MAX_DEVICES)
        return DRIVER_TOO_MANY_DEVICES;

    dev = (device_t *) kmalloc(sizeof(device_t));
    if(dev == NULL)
        return ENOMEM;

	dev->type = type;
	dev->class = class;
	dev->driver = driver;
	dev->data = data;

	/* TODO: check that the name is not a duplicate */
	strncpy(dev->name, name, sizeof(dev->name));

    g_devices[u] = dev;

	return SUCCESS;
}


driver_ret device_read(const device_t * const dev, ku32 offset, u32 len, u8 *buf)
{
	return dev->driver->read(dev->data, offset, len, buf);
}


driver_ret device_write(const device_t * const dev, ku32 offset, u32 len, ku8 *buf)
{
	return dev->driver->write(dev->data, offset, len, buf);
}


driver_ret device_control(const device_t * const dev, ku32 function, void *in, void *out)
{
	return dev->driver->control(dev->data, function, in, out);
}

