/*
	partition.c: declarations of functions and types relating to the hard disc partition model

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 9th Febrary 2012.
*/

#include "partition.h"
#include "kutil/kutil.h"
#include "include/byteorder.h"

#include "stdio.h"			/* FIXME: remove */

static struct partition_data g_partitions[MAX_PARTITIONS];
static u32 g_next_partition;

struct device_driver g_partition_driver =
{
    .name       = "part",
    .version    = 0x00000100,

    .init       = partition_init,
    .shut_down  = partition_shut_down,
    .read       = partition_read,
    .write      = partition_write,
    .control    = partition_control
};

#include "kutil/kutil.h" // FIXME remove
driver_ret partition_init()
{
	/* Scan all devices, enumerate partitions, create partition devices */
	u32 device_id;

	g_next_partition = 0;

	for(device_id = 0; device_id < MAX_DEVICES; ++device_id)
	{
		device_t * const dev = g_devices[device_id];

		if(dev && (dev->type == DEVICE_TYPE_BLOCK) && (dev->class == DEVICE_CLASS_DISC))
		{
			/* Read sector 0.  If it contains a master boot record (MBR), enumerate its partition
			 * table and create partition devices. */
			struct mbr m;
			u16 part;
			char name[DEVICE_NAME_LEN], *pn;

			if(dev->driver->read(dev->data, 0, 1, (u8 *) &m) != DRIVER_OK)
				continue;		/* Failed to read sector TODO: report error */

			if(m.mbr_signature != MBR_SIGNATURE)
				continue;		/* Sector is not a MBR */

			bzero(name, sizeof(name));
			strncpy(name, dev->name, sizeof(name) - 1);
			for(pn = name; *pn; ++pn) ;

			for(part = 0; part < MBR_NUM_PARTITIONS; ++part)
			{
				struct partition_data * const data = &g_partitions[g_next_partition++];
				struct mbr_partition * const p = &m.partition[part];
				u32 bytes_per_sector = 0;

				if((g_next_partition >= MAX_PARTITIONS) || (part > DEVICE_MAX_SUBDEVICES))
					return DRIVER_TOO_MANY_DEVICES;

				if(device_control(dev, DEVCTL_BLOCK_SIZE, NULL,
											&bytes_per_sector) != DRIVER_OK)
					continue;		/* TODO: report error */

				*pn = g_device_sub_names[part];

				data->device		= dev;
				data->sector_len	= bytes_per_sector;
				data->offset 		= wswap_32(p->first_sector_lba);
				data->len			= wswap_32(p->num_sectors);
				data->type			= p->type;
				data->status		= p->status;

				if(data->len)   /* Skip zero-length "partitions" */
                {
                    if(create_device(DEVICE_TYPE_BLOCK, DEVICE_CLASS_PARTITION, &g_partition_driver,
                                  name, data) == SUCCESS)
                        printf("%s: %uMB [%s, %s]\n", name, data->len >> (20 - LOG_BLOCK_SIZE),
                               partition_type_name(p->type), partition_status_desc(p->status));
                    else
                        printf("%s: failed to create device\n", name);
                }
			}
		}
	}

	return DRIVER_OK;
}


s8 *partition_type_name(ku8 type)
{
    switch(type)
    {
        case 0x01:
        case 0x04:
        case 0x05:
        case 0x06:
            return "DOS";

        case 0x42:
        case 0x82:
            return "Linux swap";

        case 0x43:
        case 0x83:
            return "Linux";
    }

    return "unsupported";
}


s8 *partition_status_desc(ku8 status)
{
    if(status == 0x00)
    {
        return "inactive";
    }
    else if(status >= 0x80)
    {
        return "active, bootable";
    }

    return "invalid";
}


driver_ret partition_shut_down()
{
	/* TODO: remove partition devices */

	return DRIVER_OK;
}


driver_ret partition_read(void *data, ku32 offset, ku32 len, void* buf)
{
	const struct partition_data * const part = (const struct partition_data * const) data;

	if((offset + len) > part->len)
		return DRIVER_INVALID_SEEK;

	return part->device->driver->read(part->device->data, part->offset + offset, len, buf);
}


driver_ret partition_write(void *data, ku32 offset, ku32 len, const void* buf)
{
	const struct partition_data * const part = (const struct partition_data * const) data;

	if((offset + len) > part->len)
		return DRIVER_INVALID_SEEK;

	return part->device->driver->write(part->device->data, part->offset + offset, len, buf);
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

