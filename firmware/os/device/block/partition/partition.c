/*
	partition.c: declarations of functions and types relating to the hard disc partition model

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 9th Febrary 2012.
*/

#include "partition.h"
#include "kutil/kutil.h"
#include "include/byteorder.h"

#include "stdio.h"			/* FIXME: remove */

static struct device_driver g_partition_driver;
static struct partition_data g_partitions[MAX_PARTITIONS];
static u32 g_next_partition;


struct device_driver *partition_register_driver()
{
	/* Initialisation needs to be done here (instead of through an initialisation list at
	 * the g_partition_driver definition) because the ".data" section is discarded from the
	 * firmware ROM image. */
	g_partition_driver.name			= "part";
	g_partition_driver.version		= 0x00000100;	/* v0.0.1-0 */

	g_partition_driver.init			= partition_init;
	g_partition_driver.shut_down	= partition_shut_down;

	g_partition_driver.read			= partition_read;
	g_partition_driver.write		= partition_write;

	g_partition_driver.control		= partition_control;

	return &g_partition_driver;
}


driver_ret partition_init()
{
	/* Scan all devices, enumerate partitions, create partition devices */
	u32 device_id;
	const u32 num_devices = driver_num_devices();

	g_next_partition = 0;

	for(device_id = 0; device_id < num_devices; ++device_id)
	{
		const struct device * const dev = get_device_by_devid(device_id);

printf("part: looking for partitions on %s\n", dev->name);
		if(!dev)
			continue;	/* maybe warn here - this shouldn't happen */

		if(dev->type == DEVICE_TYPE_BLOCK)
		{
			/* Read sector 0.  If it contains a master boot record (MBR), enumerate its partition
			 * table and create partition devices. */
			struct mbr m;
			u16 part;
			char name[DEVICE_NAME_LEN], *pn;

			if(!dev->driver->read)
				continue;

			if(dev->driver->read(dev->data, 0, MBR_SECTOR_LEN, (u8 *) &m) != DRIVER_OK)
				continue;		/* Failed to read sector TODO: report error */

			if(m.mbr_signature != MBR_SIGNATURE)
				continue;		/* Sector is not a MBR */

			kbzero(name, sizeof(name));
			kstrncpy(name, dev->name, sizeof(name) - 1);
			for(pn = name; *pn; ++pn) ;

			for(part = 0; part < MBR_NUM_PARTITIONS; ++part)
			{
				struct partition_data * const data = &g_partitions[g_next_partition++];
				struct mbr_partition * const p = &m.partition[part];
				u32 bytes_per_sector = 0;

				if((g_next_partition >= MAX_PARTITIONS) || (part > DEVICE_MAX_SUBDEVICES))
					return DRIVER_TOO_MANY_DEVICES;

				if(device_control(device_id, DEVCTL_BLOCK_SIZE, NULL,
											&bytes_per_sector) != DRIVER_OK)
					continue;		/* TODO: report error */

				*pn = device_sub_names[part];

				data->device		= device_id;
				data->sector_len	= bytes_per_sector;
				data->offset 		= __wswap_32(p->first_sector_lba);
				data->len			= __wswap_32(p->num_sectors);
				data->type			= p->type;
				data->status		= p->status;

printf("%s: offset=%u len=%u type=%02x status=%02x\n", name, data->offset * bytes_per_sector, 
				data->len * bytes_per_sector, data->type, data->status);

				/* Not checking retval as there's nothing we can do if this fails */
				create_device(DEVICE_TYPE_BLOCK, &g_partition_driver, name, data);
			}
		}
	}

	return DRIVER_OK;
}


driver_ret partition_shut_down()
{
	/* TODO: remove partition devices */

	return DRIVER_OK;
}


driver_ret partition_read(void *data, ku32 offset, ku32 len, void* buf)
{
	const struct partition_data * const part = (const struct partition_data * const) data;
	const struct device * const dev = get_device_by_devid(part->device);

	if(!dev)
		return DRIVER_INVALID_DEVICE;

	if(!dev->driver->read)
		return DRIVER_NOT_IMPLEMENTED;

	if((offset + len) > part->len)
		return DRIVER_INVALID_SEEK;

	return dev->driver->read(dev->data, part->offset + offset, len, buf);
}


driver_ret partition_write(void *data, ku32 offset, ku32 len, const void* buf)
{
	const struct partition_data * const part = (const struct partition_data * const) data;
	const struct device * const dev = get_device_by_devid(part->device);

	if(!dev)
		return DRIVER_INVALID_DEVICE;

	if(!dev->driver->write)
		return DRIVER_NOT_IMPLEMENTED;

	if((offset + len) > part->len)
		return DRIVER_INVALID_SEEK;

	return dev->driver->write(dev->data, part->offset + offset, len, buf);
}


driver_ret partition_control(void *data, ku32 function, void *in, void *out)
{
	const struct partition_data * const part = (const struct partition_data * const) data;

	switch(function)
	{
		case DEVCTL_EXTENT:
			*((u32 *) out) = part->len;
			break;

		case DEVCTL_BLOCK_SIZE:
			*((u32 *) out) = part->sector_len;
			break;

		case DEVCTL_BOOTABLE:
			*((u32 *) out) = (part->status == PARTITION_STATUS_BOOTABLE);
			break;

		default:
			return DRIVER_NOT_IMPLEMENTED;
	}

	return DRIVER_OK;
}

