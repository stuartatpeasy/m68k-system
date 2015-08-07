/*
	Virtual file system abstraction

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, July 2012.
*/

#include "fs/vfs.h"
#include "fs/mount.h"
#include "device/device.h"
#include "device/devctl.h"


device_t *find_boot_device()
{
	u32 i;

	for(i = 0; i < MAX_DEVICES; ++i)
	{
        if(g_devices[i] != NULL)
        {
            device_t * const dev = g_devices[i];
            if(dev->type == DEVICE_TYPE_BLOCK)
            {
                u32 bootable = 0;

                if(device_control(dev, DEVCTL_BOOTABLE, NULL, &bootable) == SUCCESS)
                {
                    if(bootable)
                        return dev;
                }
            }
        }
	}

	return NULL;
}


u32 vfs_init()
{
	/* Find boot device */
	device_t *boot_device;

	boot_device = find_boot_device();

	if(!boot_device)
	{
		/* FIXME: once kprintf() is working, report no boot dev and drop to monitor */
		printf("No bootable partitions found\n");

		return FAIL;
	}

	printf("Boot device: %s\n", boot_device->name);

	/* Mount the boot device at / */
	mount_add("/", boot_device);

	return SUCCESS;
}
