/*
	partition.c: declarations of functions and types relating to the hard disc partition model

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 9th February 2012.
*/

#include <include/byteorder.h>
#include <include/mbr.h>
#include <kernel/device/partition.h>
#include <kernel/util/kutil.h>

#include <strings.h>


/* Module-private function declarations */
s32 partition_control(dev_t *dev, ku32 function, const void *in, void *out);
s32 partition_read(dev_t *dev, ku32 offset, u32 *len, void* buf);
s32 partition_write(dev_t *dev, ku32 offset, u32 *len, const void* buf);


/*
    partition_init() - scan all mass-storage devices and attempt to find partitions.  Create
    partition devices as required.
*/
s32 partition_init()
{
    dev_t *dev = NULL;
    u32 one = 1;

    /* Find mass-storage devices */
    while((dev = dev_get_next(dev)) != NULL)
    {
        if((dev->type != DEV_TYPE_BLOCK) || (dev->subtype != DEV_SUBTYPE_MASS_STORAGE))
            continue;

        /*
            Read sector 0.  If it contains a master boot record (MBR), enumerate its partition
            table and create partition devices.
        */
        struct mbr m;
        u16 part;

        if(dev->read(dev, 0, &one, (u8 *) &m) != SUCCESS)
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

            if(dev->control(dev, dc_get_block_size, NULL, &bytes_per_sector) != SUCCESS)
                continue;		/* TODO: report error */

            data = CHECKED_KCALLOC(1, sizeof(partition_data_t));

            if(dev_register(DEV_TYPE_BLOCK, DEV_SUBTYPE_PARTITION, dev->name, IRQL_NONE, NULL,
                            &part_dev, "partition", dev, NULL) != SUCCESS)
            {
                kfree(data);
                continue;
            }

            part_dev->read          = partition_read;
            part_dev->write         = partition_write;
            part_dev->control       = partition_control;
            part_dev->shut_down     = partition_shut_down;
            part_dev->block_size    = dev->block_size;
            part_dev->len           = LE2N32(p->num_sectors);

            data->device		= dev;
            data->block_size	= bytes_per_sector;
            data->offset 		= LE2N32(p->first_sector_lba);
            data->type			= p->type;
            data->status		= p->status;

            part_dev->data = data;
        }
    }

	return SUCCESS;
}


/*
    partition_type_name() - given a "type" byte, return a human-readable name for the type.
*/
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


/*
    partition_status_desc() - given a "status" flag, return a human-readable description.
*/
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


/*
    partition_shut_down() - perform shutdown tasks prior to deleting the partition device object.
*/
s32 partition_shut_down(dev_t *dev)
{
    kfree(dev->data);

	return SUCCESS;
}


/*
    partition_read() - read len blocks from a partition, starting at offset.
*/
s32 partition_read(dev_t *dev, ku32 offset, u32 *len, void* buf)
{
    partition_data_t * const part_data = (partition_data_t *) dev->data;
    dev_t * const block_dev = part_data->device;

	if((offset + *len) > dev->len)
		return EINVAL;

    return block_dev->read(block_dev, part_data->offset + offset, len, buf);
}


/*
    partition_write() - write len blocks to a partition, starting at offset.
*/
s32 partition_write(dev_t *dev, ku32 offset, u32 *len, const void* buf)
{
    partition_data_t * const part_data = (partition_data_t *) dev->data;
    dev_t * const block_dev = part_data->device;

	if((offset + *len) > dev->len)
		return EINVAL;

    return block_dev->write(block_dev, part_data->offset + offset, len, buf);
}


/*
    partition_control() - devctl handler.
*/
s32 partition_control(dev_t *dev, const devctl_fn_t fn, const void *in, void *out)
{
	const struct partition_data * const pdata = (const struct partition_data * const) dev->data;
    UNUSED(in);

	switch(fn)
	{
		case dc_get_extent:
			*((u32 *) out) = dev->len;
			break;

		case dc_get_block_size:
			*((u32 *) out) = dev->block_size;
			break;

		case dc_get_bootable:
			*((u32 *) out) = (pdata->status == PARTITION_STATUS_BOOTABLE);
			break;

        case dc_get_model:
            *((s8 **) out) = "partition";
            break;

        case dc_get_partition_type:
            *((u32 *) out) = pdata->type;
            break;

        case dc_get_partition_type_name:
            *((s8 **) out) = partition_type_name(pdata->type);
            break;

        case dc_get_partition_active:
            *((u32 *) out) = (pdata->status >= 0x80);
            break;

		default:
			return ENOSYS;
	}

	return SUCCESS;
}

