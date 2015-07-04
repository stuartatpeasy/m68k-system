/*
	device.c: device and device driver management

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 9th Febrary 2012.
*/

#include "stdio.h"			/* FIXME: remove */

#include "kutil/kutil.h"
#include "kutil/bvec.h"
#include "device/device.h"

/* Characters used to identify "sub-devices", e.g. partitions of devices.  The first sub-device
 * of device xxx will be xxx1, the second xxx2, ..., the 61st xxxa.Z */
const char * const device_sub_names = "123456789abcdefghijklmnopqrstuv"
									  "wxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

bvec_t g_devices;
bvec_t g_drivers;

/* #include driver registration function declarations here */
#include "device/block/ata/ata.h"
#include "device/block/partition/partition.h"

/* List all device driver registration functions in this array */
device_driver_t *(* const g_driver_registerers[])() =
{
	ata_register_driver,

	/* NB: all drivers of partitionable devices must be registered before the partition driver! */
	partition_register_driver
};


u32 driver_init()
{
	const u32 num_drivers = sizeof(g_driver_registerers) / sizeof(device_driver_t *(* const)());
	u32 x;

	/* TODO: remove hardwired const */
	x = bvec_init(8 /* block size */, sizeof(device_t), &g_devices);
	if(x)
		return x;	/* probably ENOMEM - should not happen */

	/* TODO: remove hardwired const */
	x = bvec_init(8 /* block size */, sizeof(device_driver_t *), &g_drivers);
	if(x)
		return x;	/* probably ENOMEM - should not happen */


	/* FIXME: wtf is this? Surely it is broken and can be removed */
	kbzero(&g_driver_registerers, sizeof(g_driver_registerers));

	for(x = 0; x < num_drivers; ++x)
	{
		const device_driver_t *drv = g_driver_registerers[x]();

		if(drv)
		{
			const device_driver_t ** p;

			printf("'%s' driver registered\n", drv->name);

			if(drv->init() != DRIVER_OK)	/* init() will create devices */
			{
				/* TODO: error */
				printf("%s: device init failed\n", drv->name);
				continue;
			}

			p = bvec_grow(g_drivers);
			if(p == NULL)
			{
				printf("Failed to register driver - ENOMEM\n");		/* TODO: improve this */
				continue;
			}

			*p = drv;
		}
		else
		{
			/* TODO: error */
		}
	}

	return SUCCESS;
}


void driver_shut_down()
{
	/* TODO: ensure all devices are stopped/flushed/etc */
	int i;
	for(i = 0; i < bvec_size(g_drivers); ++i)
	{
printf("Shutting down driver %i\n", i);
		if(((const device_driver_t *) g_drivers->elements[i])->shut_down() != DRIVER_OK)
		{
			/* TODO: error */
kputs("--- shut down failed");
		}
	}

	bvec_destroy(&g_drivers);
	bvec_destroy(&g_devices);
}


u32 driver_num_devices()
{
	return bvec_size(g_devices);
}


const device_t *get_device_by_devid(const device_id id)
{
	/* TODO: is this function public? if not, it can be deleted */
	return bvec_get(g_devices, id);	/* will return NULL if no such device */
}


const device_t *get_device_by_name(const char * const name)
{
	int i;
	for(i = 0; i < bvec_size(g_devices); ++i)
	{
		if(!kstrcmp(((device_t *) g_devices->elements[i])->name, name))
			return g_devices->elements[i];
	}

	return NULL;	/* No such device */
}


/* Create (i.e. register) a new device.  This function is called by each driver's init() function
 * during device enumeration. */
driver_ret create_device(const enum device_type type, device_driver_t * const driver, 
							const char *name, void * const data)
{
	device_t * const dev = bvec_grow(g_devices);

	if(dev == NULL)
		return ENOMEM;

	dev->type = type;
	dev->driver = driver;
	dev->data = data;

	/* TODO: check that the name is not a duplicate */
	kstrncpy(dev->name, name, sizeof(dev->name));

	return SUCCESS;
}


driver_ret device_read(const device_id id, ku32 offset, u32 len, u8 *buf)
{
	device_t * const dev = bvec_get(g_devices, id);

	if(dev == NULL)
		return ENODEV;	/* no such device */
	
	if(!dev->driver->read)
		return ENOSYS;	/* not implemented */

	return dev->driver->read(dev->data, offset, len, buf);
}


driver_ret device_write(const device_id id, ku32 offset, u32 len, ku8 *buf)
{
	device_t * const dev = bvec_get(g_devices, id);

	if(dev == NULL)
		return ENODEV;	/* no such device */
	
	if(!dev->driver->write)
		return ENOSYS;	/* not implemented */
	
	return dev->driver->write(dev->data, offset, len, buf);
}


driver_ret device_control(const device_id id, ku32 function, void *in, void *out)
{
	device_t * const dev = bvec_get(g_devices, id);

	if(dev == NULL)
		return ENODEV;	/* no such device */

	if(!dev->driver->control)
		return ENOSYS;	/* not implemented */
	
	return dev->driver->control(dev->data, function, in, out);
}

