/*
	partition.c: declarations of functions and types relating to the hard disc partition model

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 9th February 2012.
*/

#include <device/partition.h>
#include <include/byteorder.h>
#include <include/mbr.h>
#include <kutil/kutil.h>

#include <stdio.h>
#include <strings.h>


s32 partition_init()
{
    dev_t *dev = NULL;

    /* Find mass-storage devices */
    while((dev = dev_get_next(dev)) != NULL)
    {
        if((dev->type == DEV_TYPE_BLOCK) && (dev->subtype == DEV_SUBTYPE_MASS_STORAGE))
        {
			/*
                Read sector 0.  If it contains a master boot record (MBR), enumerate its partition
                table and create partition devices.
            */
			struct mbr m;
			u16 part;

			if(((block_ops_t *) dev->driver)->read(dev, 0, 1, (u8 *) &m) != SUCCESS)
				continue;		/* Failed to read sector TODO: report error */

			if(LE2N16(m.mbr_signature) != MBR_SIGNATURE)
				continue;		/* Sector is not a MBR */

			for(part = 0; part < MBR_NUM_PARTITIONS; ++part)
			{
				struct mbr_partition * const p = &m.partition[part];
				u32 bytes_per_sector = 0;
				partition_data_t * data;
				dev_t *part_dev;

				if(part > DEVICE_MAX_SUBDEVICES)
					return ENFILE;

                if(!p->num_sectors)
                    continue;       /* Skip zero-length partitions */

				if(device_control(dev, DEVCTL_BLOCK_SIZE, NULL,
                                    &bytes_per_sector) != SUCCESS)
					continue;		/* TODO: report error */

                data = CHECKED_KCALLOC(1, sizeof(partition_data_t));

                if(dev_register(DEV_TYPE_BLOCK, DEV_SUBTYPE_PARTITION, dev->name, IRQL_NONE, NULL,
                                &part_dev, "partition", dev, NULL) == SUCCESS)
                {
                    block_ops_t *ops = kcalloc(1, sizeof(block_ops_t));

                    ops->read = partition_read;
                    ops->write = partition_write;
                    ops->control = partition_control;

                    part_dev->driver = ops;

                    data->device		= dev;
                    data->sector_len	= bytes_per_sector;
                    data->offset 		= LE2N32(p->first_sector_lba);
                    data->len			= LE2N32(p->num_sectors);
                    data->type			= p->type;
                    data->status		= p->status;

                    part_dev->data = data;

                    printf("%s: %4uMB [%s, %s]\n", part_dev->name,
                           LE2N32(p->num_sectors) >> (20 - LOG_BLOCK_SIZE),
                           partition_type_name(p->type), partition_status_desc(p->status));
                }
                else
                {
                    kfree(data);
                    printf("%s: failed to create device for partition %u", dev->name, part);
                }
			}
        }
    }


	return SUCCESS;
}


s8 *partition_type_name(ku8 type)
{
    switch(type)
    {
        case 0x01:
        case 0x04:
        case 0x05:
        case 0x06:
            return "MS-DOS";

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


s32 partition_shut_down()
{
	/* TODO: remove partition devices */

	return SUCCESS;
}


s32 partition_read(dev_t *dev, ku32 offset, ku32 len, void* buf)
{
    partition_data_t * const part_data = (partition_data_t *) dev->data;
    dev_t * const block_dev = part_data->device;
    block_ops_t *const ops = (block_ops_t *) block_dev->driver;

	if((offset + len) > part_data->len)
		return EINVAL;

    return ops->read(block_dev, part_data->offset + offset, len, buf);
}


s32 partition_write(dev_t *dev, ku32 offset, ku32 len, const void* buf)
{
    partition_data_t * const part_data = (partition_data_t *) dev->data;
    dev_t * const block_dev = part_data->device;
    block_ops_t *const ops = (block_ops_t *) block_dev->driver;

	if((offset + len) > part_data->len)
		return EINVAL;

    return ops->write(block_dev, part_data->offset + offset, len, buf);
}


s32 partition_control(dev_t *dev, ku32 function, void *in, void *out)
{
	const struct partition_data * const pdata = (const struct partition_data * const) dev->data;

	switch(function)
	{
		case DEVCTL_EXTENT:
			*((u32 *) out) = pdata->len;
			break;

		case DEVCTL_BLOCK_SIZE:
			*((u32 *) out) = pdata->sector_len;
			break;

		case DEVCTL_BOOTABLE:
			*((u32 *) out) = (pdata->status == PARTITION_STATUS_BOOTABLE);
			break;

        case DEVCTL_MODEL:
            *((s8 **) out) = "partition";

		default:
			return ENOSYS;
	}

	return SUCCESS;
}

