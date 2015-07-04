/*
	Virtual file system abstraction

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, July 2012.
*/

#include "fs/vfs.h"
#include "fs/mount.h"
#include "device/device.h"
#include "device/devctl.h"


device_id find_boot_device()
{
	u32 i;
	const struct device *dev;

	for(i = 0; (dev = get_device_by_devid(i)); ++i)
	{
		if(dev->type == DEVICE_TYPE_BLOCK)
		{
			u32 bootable = 0;

			if(device_control(i, DEVCTL_BOOTABLE, NULL, &bootable) == DRIVER_OK)
			{
				if(bootable)
					return i;
			}
		}
	}

	return INVALID_DEVICE_ID;
}


u32 vfs_init()
{
	/* Find boot device */
	device_id boot_device;

	if(mount_init())
		return FAIL;

	boot_device = find_boot_device();
	if(boot_device == INVALID_DEVICE_ID)
	{
		/* FIXME: once kprintf() is working, report no boot dev and drop to monitor */
		printf("No bootable partitions found\n");

		return FAIL;
	}

	printf("Boot device: %s\n", get_device_by_devid(boot_device)->name);

	/* Mount the boot device at / */
	mount_add("/", boot_device);

	return SUCCESS;
}
