/*
	AT Attachment (ATA) interface driver function definitions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, December 2011.

	TODO: remove libc-sw functions (bcopy, strncpy, etc)
*/

#include "ata-internal.h"
#include "ata.h"
#include "include/error.h"


static ata_device_t g_ata_devices[ATA_MAX_DEVICES];

struct device_driver g_ata_driver =
{
    .name       = "ata",
    .version    = 0x00000100,

    .init       = ata_init,
    .shut_down  = ata_shut_down,
    .read       = ata_read,
    .write      = ata_write,
    .control    = ata_control
};


/*
	Initialise ATA interface
*/
s32 ata_init()
{
	int bus;
	char device_name[DEVICE_NAME_LEN], *dn;

	bzero(g_ata_devices, sizeof(g_ata_devices));
	bzero(device_name, DEVICE_NAME_LEN);

	/* The first ATA device is called "ataa", the second "atab", etc.  device_name will pass the
	 * name of each device to the create_device().  device_name is initialised to "ata"; dn
	 * points to the next char after "ata".  This makes it easy to form the proper device names */
	strcpy(device_name, g_ata_driver.name);
	for(dn = device_name; *dn; ++dn) ;

	for(bus = 0; bus < ATA_NUM_BUSES; ++bus)
	{
		int device;

        /* Ensure that the software reset flag is cleared for the bus, and disable interrupts */
        ATA_REG(bus, ATA_R_DEVICE_CONTROL) |= ATA_DEVICE_CONTROL_NIEN;
        ATA_REG(bus, ATA_R_DEVICE_CONTROL) &= ~ATA_DEVICE_CONTROL_SRST;

		/* Attempt to detect the master device (device 0) first */
		ATA_REG(bus, ATA_R_DEVICE_HEAD) &= ~ATA_DH_DEV;
		for(device = 0; device < ATA_DEVICES_PER_BUS;
				++device, ATA_REG(bus, ATA_R_DEVICE_HEAD) |= ATA_DH_DEV)
		{
			struct ata_identify_device_ret id;
			ata_device_t * const devp = &g_ata_devices[(bus * ATA_DEVICES_PER_BUS) + device];

			/* Wait for BSY to become low */
			if(!ATA_WAIT_NBSY(bus))
			{
				devp->status = timeout;
				continue;
			}

			ATA_REG(bus, ATA_R_COMMAND) = ATA_CMD_IDENTIFY_DEVICE;

			/* Wait for BSY to go high */
			if(!ATA_WAIT_BSY(bus))
				continue;	/* No response from device; presumably nothing plugged in to the port */

			/* A device is present. Wait for BSY to go low */
			if(!ATA_WAIT_NBSY(bus))
			{
				devp->status = timeout;
				continue;	/* Device did not negate BSY in a timely fashion */
			}

			/* Did an error occur? */
			if(ATA_REG(bus, ATA_R_STATUS) & ATA_STATUS_ERR)
			{
				/* Was the command aborted? */
				if(ATA_REG(bus, ATA_R_ERROR) & ATA_ERROR_ABRT)
				{
					/* This might be a packet device. */
					if((ATA_REG(bus, ATA_R_SECTOR_COUNT) == 0x01) &&		/* \ this is the	*/
					   (ATA_REG(bus, ATA_R_SECTOR_NUMBER) == 0x01) &&		/* | PACKET command	*/
					   (ATA_REG(bus, ATA_R_CYLINDER_LOW) == 0x14) &&		/* | feature set	*/
					   (ATA_REG(bus, ATA_R_CYLINDER_HIGH) == 0xeb) &&		/* | signature		*/
					   ((ATA_REG(bus, ATA_R_DEVICE_HEAD) & 0xef) == 0x00))	/* /				*/
					{
						/* This is a packet device; these are not supported. */
						devp->status = unsupported;
						continue;
					}
				}

				/* Some other failure occurred; shouldn't happen during IDENTIFY DEVICE. Hey ho. */
				devp->status = failed;
				continue;
			}

			ata_read_data(devp->addr.bus, &id);

			devp->addr.bus = bus;
			devp->addr.device = device;

            strn_trim_cpy(devp->model, id.model_number, sizeof(id.model_number));
            strn_trim_cpy(devp->serial, id.serial_number, sizeof(id.serial_number));
            strn_trim_cpy(devp->firmware, id.firmware_revision, sizeof(id.firmware_revision));

			devp->num_sectors = id.log_cyls * id.log_heads * id.log_sects_per_log_track;

			*dn = g_device_sub_names[(bus * ATA_DEVICES_PER_BUS) + device];
			devp->status = online;

			if(create_device(DEVICE_TYPE_BLOCK, DEVICE_CLASS_DISC, &g_ata_driver, device_name,
                             devp) == SUCCESS)
                printf("%s: %s, %uMB [serial %s firmware %s]\n", device_name, devp->model,
                       devp->num_sectors >> 11, devp->serial, devp->firmware);
            else
                printf("%s: failed to create device\n", device_name);
		}
	}

	return SUCCESS;
}


s32 ata_shut_down(void)
{
	bzero(g_ata_devices, sizeof(g_ata_devices));

	return SUCCESS;
}


/*
	Send ATA command
*/
ata_ret_t ata_send_command(const u32 devid, const ata_command_t cmd)
{
	if((devid > ATA_MAX_DEVICES) || g_ata_devices[devid].status != online)
		return ENODEV;

	const ata_device_t *devp = &g_ata_devices[devid];

	/* Select bus master / slave device */
	if(devp->addr.device == 0)
		ATA_REG(devp->addr.bus, ATA_R_DEVICE_HEAD) &= ~ATA_DH_DEV;
	else if(devp->addr.device == 1)
		ATA_REG(devp->addr.bus, ATA_R_DEVICE_HEAD) |= ATA_DH_DEV;
	else
		return ENODEV;

	/* Wait for the device to become ready */
	if(!ATA_WAIT_NBSY(devp->addr.bus))
		return ETIME;

	ATA_REG(devp->addr.bus, ATA_R_COMMAND) = cmd;

	/* Wait for the command to complete */
	if(!ATA_WAIT_NBSY(devp->addr.bus))
		return ETIME;

	/* Did an error occur? */
	if(ATA_REG(devp->addr.bus, ATA_R_STATUS) & ATA_STATUS_ERR)
	{
		/* FIXME: correctly report the error */
		return EUNKNOWN;
	}

	return SUCCESS;
}


s32 ata_read(void *data, ku32 offset, u32 len, void * buf)
{
	const ata_device_t * const devp = (const ata_device_t * const) data;

	if(devp->status != online)
		return ENODEV;

	/* Select bus master / slave device */
	if(devp->addr.device == 0)
		ATA_REG(devp->addr.bus, ATA_R_DEVICE_HEAD) &= ~ATA_DH_DEV;
	else if(devp->addr.device == 1)
		ATA_REG(devp->addr.bus, ATA_R_DEVICE_HEAD) |= ATA_DH_DEV;
	else
		return ENODEV;

	if(!ATA_WAIT_NBSY(devp->addr.bus))
		return ETIME;

	if((offset + len < offset)
	   || (offset + len > devp->num_sectors))
	   return EINVAL;

	/* TODO: check that requested # sectors is allowed by the device; split into smaller reads if not. */

	ATA_REG(devp->addr.bus, ATA_R_SECTOR_COUNT)		= len;

	ATA_REG(devp->addr.bus, ATA_R_SECTOR_NUMBER)	= offset & 0xff;
	ATA_REG(devp->addr.bus, ATA_R_CYLINDER_LOW)		= (offset >> 8) & 0xff;
	ATA_REG(devp->addr.bus, ATA_R_CYLINDER_HIGH)	= (offset >> 16) & 0xff;
	ATA_REG(devp->addr.bus, ATA_R_DEVICE_HEAD)	   |= ATA_DH_LBA | ((offset >> 24) & 0xf);

	/* Issue the command */
	ATA_REG(devp->addr.bus, ATA_R_COMMAND) = ATA_CMD_READ_SECTORS;

	if(!ATA_WAIT_NBSY(devp->addr.bus))
		return ETIME;

	if(ATA_REG(devp->addr.bus, ATA_R_STATUS) & ATA_STATUS_ERR)
	{
		/* FIXME: correctly report the error */
		return EUNKNOWN;
	}

	while(len--)
	{
		ata_read_data(devp->addr.bus, buf);
		buf += ATA_SECTOR_SIZE;
	}

	return SUCCESS;
}


s32 ata_write(void *data, ku32 offset, u32 len, const void * buf)
{
	const ata_device_t * const devp = (const ata_device_t * const) data;

	if(devp->status != online)
		return ENODEV;

	/* Select bus master / slave device */
	if(devp->addr.device == 0)
		ATA_REG(devp->addr.bus, ATA_R_DEVICE_HEAD) &= ~ATA_DH_DEV;
	else if(devp->addr.device == 1)
		ATA_REG(devp->addr.bus, ATA_R_DEVICE_HEAD) |= ATA_DH_DEV;
	else
		return ENODEV;

	if(!ATA_WAIT_NBSY(devp->addr.bus))
		return ETIME;

	if((offset + len < offset)
	   || (offset + len > devp->num_sectors))
	   return EINVAL;

	/* TODO: check that requested # sectors is allowed by the device; split into smaller reads if not. */

	ATA_REG(devp->addr.bus, ATA_R_SECTOR_COUNT)		= len;

	ATA_REG(devp->addr.bus, ATA_R_SECTOR_NUMBER)	= offset & 0xff;
	ATA_REG(devp->addr.bus, ATA_R_CYLINDER_LOW)		= (offset >> 8) & 0xff;
	ATA_REG(devp->addr.bus, ATA_R_CYLINDER_HIGH)	= (offset >> 16) & 0xff;
	ATA_REG(devp->addr.bus, ATA_R_DEVICE_HEAD)	   |= ATA_DH_LBA | ((offset >> 24) & 0xf);

	/* Issue the command */
	ATA_REG(devp->addr.bus, ATA_R_COMMAND) = ATA_CMD_WRITE_SECTORS;

	while(len--)
	{
		ata_write_data(devp->addr.bus, buf);
		buf += ATA_SECTOR_SIZE;
	}

	if(!ATA_WAIT_NBSY(devp->addr.bus))
		return ETIME;

	/* Did an error occur? */
	if(ATA_REG(devp->addr.bus, ATA_R_STATUS) & ATA_STATUS_ERR)
	{
		/* FIXME: correctly report the error */
		return EUNKNOWN;
	}

	return SUCCESS;
}


/*
	Read data buffer
*/
void ata_read_data(ku32 bus, void *buf)
{
	u16 *buf_ = buf;
	u32 count = (ATA_SECTOR_SIZE / sizeof(u16));

	for(; count--;)
		*buf_++ = ATA_REG_DATA(bus);
}


/*
	Write data buffer
*/
void ata_write_data(ku32 bus, const void *buf)
{
	const u16 *buf_ = buf;
	u32 count = (ATA_SECTOR_SIZE / sizeof(u16));

	for(; count--;)
		ATA_REG_DATA(bus) = *buf_++;
}


/*
	Device control
*/
s32 ata_control(void *data, ku32 function, void *in, void *out)
{
	ata_device_t * const devp = (ata_device_t * const) data;

	switch(function)
	{
		case DEVCTL_EXTENT:
			*((u32 *) out) = devp->num_sectors;
			break;

		case DEVCTL_BLOCK_SIZE:
			*((u32 *) out) = ATA_SECTOR_SIZE;
			break;

		case DEVCTL_BOOTABLE:
			*((u32 *) out) = 0;		/* partitions might be bootable; the device isn't */
			break;

        case DEVCTL_MODEL:
            *((s8 **) out) = devp->model;
            break;

        case DEVCTL_SERIAL:
            *((s8 **) out) = devp->serial;
            break;

        case DEVCTL_FIRMWARE_VER:
            *((s8 **) out) = devp->firmware;
            break;

		default:
			return ENOSYS;
	}

	return SUCCESS;
}

