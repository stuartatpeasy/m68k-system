/*
	AT Attachment (ATA) interface driver function definitions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, December 2011.

	TODO: remove libc-sw functions (bcopy, strncpy, etc)
*/

#include "ata-internal.h"
#include "ata.h"
#include "kutil/kutil.h"

#include "stdio.h"			/* FIXME: remove this */


static ata_device_t g_devices[ATA_MAX_DEVICES];
static struct device_driver g_ata_driver;


struct device_driver *ata_register_driver()
{
	/* Initialisation needs to be done here (instead of through an initialisation list at
	 * the g_partition_driver definition) because the ".data" section is discarded from the
	 * firmware ROM image. */
	g_ata_driver.name		= "ata";
	g_ata_driver.version	= 0x00000100;	/* v0.0.1-0 */

	g_ata_driver.init		= ata_init;
	g_ata_driver.shut_down	= ata_shut_down;

	g_ata_driver.read		= ata_read;
	g_ata_driver.write		= ata_write;

	g_ata_driver.control	= ata_control;

	return &g_ata_driver;
}


/*
	Initialise ATA interface
*/
driver_ret ata_init()
{
	int bus, devid;
	char device_name[DEVICE_NAME_LEN], *dn;

	kbzero(g_devices, sizeof(g_devices));
	kbzero(device_name, DEVICE_NAME_LEN);

	/* The first ATA device is called "ataa", the second "atab", etc.  device_name will pass the
	 * name of each device to the create_device().  device_name is initialised to "ata"; dn
	 * points to the next char after "ata".  This makes it easy to form the proper device names */
	kstrcpy(device_name, g_ata_driver.name);
	for(dn = device_name; *dn; ++dn) ;

	for(bus = 0, devid = 0; bus < ATA_NUM_BUSES; ++bus)
	{
		int device;

        /* Ensure that the software reset flag is cleared for the bus, and disable interrupts */
        ATA_REG(bus, ATA_R_DEVICE_CONTROL) |= ATA_DEVICE_CONTROL_NIEN;
        ATA_REG(bus, ATA_R_DEVICE_CONTROL) &= ~ATA_DEVICE_CONTROL_SRST;

		/* Attempt to detect the master device first */
		ATA_REG(bus, ATA_R_DEVICE_HEAD) &= ATA_DH_DEV;
		for(device = 0; device < ATA_DEVICES_PER_BUS;
				++device, ATA_REG(bus, ATA_R_DEVICE_HEAD) |= ATA_DH_DEV)
		{
			struct ata_identify_device_ret id;
			ata_device_t * const devp = &g_devices[devid++];

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
					if((ATA_REG(bus, ATA_R_SECTOR_COUNT) == 0x01) &&			/* \ this is the	*/
					   (ATA_REG(bus, ATA_R_SECTOR_NUMBER) == 0x01) &&			/* | PACKET command	*/
					   (ATA_REG(bus, ATA_R_CYLINDER_LOW) == 0x14) &&			/* | feature set	*/
					   (ATA_REG(bus, ATA_R_CYLINDER_HIGH) == 0xeb) &&			/* | signature		*/
					   ((ATA_REG(bus, ATA_R_DEVICE_HEAD) & 0xef) == 0x00))		/* /				*/
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

			kstrncpy(devp->model, id.model_number, sizeof(id.model_number));
			kstrncpy(devp->serial, id.serial_number, sizeof(id.serial_number));
			kstrncpy(devp->firmware, id.firmware_revision, sizeof(id.firmware_revision));

			devp->num_sectors = id.log_cyls * id.log_heads * id.log_sects_per_log_track;

			*dn = device_sub_names[devid];
			devp->status = online;

			/* retval not checked as there's nothing we can do if this fails */
printf("ata: creating device '%s' (%uMB)\n", device_name, devp->num_sectors >> 11);
			create_device(DEVICE_TYPE_BLOCK, &g_ata_driver, device_name, devp);
		}
	}

	return DRIVER_OK;
}


driver_ret ata_shut_down(void)
{
	kbzero(g_devices, sizeof(g_devices));

	return DRIVER_OK;
}


/*
	Send ATA command
*/
ata_ret_t ata_send_command(const u32 devid, const ata_command_t cmd)
{
	if((devid > ATA_MAX_DEVICES) || g_devices[devid].status != online)
		return DRIVER_INVALID_DEVICE;

	const ata_device_t *devp = &g_devices[devid];

	/* Select bus master / slave device */
	if(devp->addr.device == 0)
		ATA_REG(devp->addr.bus, ATA_R_DEVICE_HEAD) &= ~ATA_DH_DEV;
	else if(devp->addr.device == 1)
		ATA_REG(devp->addr.bus, ATA_R_DEVICE_HEAD) |= ATA_DH_DEV;
	else
		return DRIVER_INVALID_DEVICE;

	/* Wait for the device to become ready */
	if(!ATA_WAIT_NBSY(devp->addr.bus))
		return DRIVER_TIMEOUT;

	ATA_REG(devp->addr.bus, ATA_R_COMMAND) = cmd;

	/* Wait for the command to complete */
	if(!ATA_WAIT_NBSY(devp->addr.bus))
		return DRIVER_TIMEOUT;

	/* Did an error occur? */
	if(ATA_REG(devp->addr.bus, ATA_R_STATUS) & ATA_STATUS_ERR)
	{
		/* FIXME: correctly report the error */
		return DRIVER_UNKNOWN_ERROR;
	}

	return DRIVER_OK;
}


driver_ret ata_read(void *data, ku32 offset, u32 len, void *buf)
{
	const ata_device_t * const devp = (const ata_device_t * const) data;

	/* This is a block device, so we can assume that offset and len will be properly aligned */
	const u32 first_sector = offset >> ATA_LOG_SECTOR_SIZE;
	u32 num_sectors = (len + (ATA_SECTOR_SIZE - 1)) >> ATA_LOG_SECTOR_SIZE;

	if(devp->status != online)
		return DRIVER_INVALID_DEVICE;

	/* Select bus master / slave device */
	if(devp->addr.device == 0)
		ATA_REG(devp->addr.bus, ATA_R_DEVICE_HEAD) &= ~ATA_DH_DEV;
	else if(devp->addr.device == 1)
		ATA_REG(devp->addr.bus, ATA_R_DEVICE_HEAD) |= ATA_DH_DEV;
	else
		return DRIVER_INVALID_DEVICE;

	if(!ATA_WAIT_NBSY(devp->addr.bus))
		return DRIVER_TIMEOUT;

	if((first_sector + num_sectors < first_sector)
	   || (first_sector + num_sectors > devp->num_sectors))
	   return DRIVER_INVALID_SEEK;

	/* TODO: check that requested # sectors is allowed by the device; split into smaller reads if not. */

	ATA_REG(devp->addr.bus, ATA_R_SECTOR_COUNT)		= num_sectors;

	ATA_REG(devp->addr.bus, ATA_R_SECTOR_NUMBER)	= first_sector & 0xff;
	ATA_REG(devp->addr.bus, ATA_R_CYLINDER_LOW)		= (first_sector >> 8) & 0xff;
	ATA_REG(devp->addr.bus, ATA_R_CYLINDER_HIGH)	= (first_sector >> 16) & 0xff;
	ATA_REG(devp->addr.bus, ATA_R_DEVICE_HEAD)	   |= ATA_DH_LBA | ((first_sector >> 24) & 0xf);

	/* Issue the command */
	ATA_REG(devp->addr.bus, ATA_R_COMMAND) = ATA_CMD_READ_SECTORS;

	if(!ATA_WAIT_NBSY(devp->addr.bus))
		return DRIVER_TIMEOUT;

	if(ATA_REG(devp->addr.bus, ATA_R_STATUS) & ATA_STATUS_ERR)
	{
		/* FIXME: correctly report the error */
		return DRIVER_UNKNOWN_ERROR;
	}

	while(num_sectors--)
	{
		ata_read_data(devp->addr.bus, buf);
		buf += ATA_SECTOR_SIZE;
	}

	return DRIVER_OK;
}


driver_ret ata_write(void *data, ku32 offset, ku32 len, const void* buf)
{
	const ata_device_t * const devp = (const ata_device_t * const) data;

	/* This is a block device, so we can assume that offset and len will be properly aligned */
	const u32 first_sector = offset >> ATA_LOG_SECTOR_SIZE;
	u32 num_sectors = (len + (ATA_SECTOR_SIZE - 1)) >> ATA_LOG_SECTOR_SIZE;

	if(devp->status != online)
		return DRIVER_INVALID_DEVICE;

	/* Select bus master / slave device */
	if(devp->addr.device == 0)
		ATA_REG(devp->addr.bus, ATA_R_DEVICE_HEAD) &= ~ATA_DH_DEV;
	else if(devp->addr.device == 1)
		ATA_REG(devp->addr.bus, ATA_R_DEVICE_HEAD) |= ATA_DH_DEV;
	else
		return DRIVER_INVALID_DEVICE;

	if(!ATA_WAIT_NBSY(devp->addr.bus))
		return DRIVER_TIMEOUT;

	if((first_sector + num_sectors < first_sector)
	   || (first_sector + num_sectors > devp->num_sectors))
	   return DRIVER_INVALID_SEEK;

	/* TODO: check that requested # sectors is allowed by the device; split into smaller reads if not. */

	ATA_REG(devp->addr.bus, ATA_R_SECTOR_COUNT)		= num_sectors;

	ATA_REG(devp->addr.bus, ATA_R_SECTOR_NUMBER)	= first_sector & 0xff;
	ATA_REG(devp->addr.bus, ATA_R_CYLINDER_LOW)		= (first_sector >> 8) & 0xff;
	ATA_REG(devp->addr.bus, ATA_R_CYLINDER_HIGH)	= (first_sector >> 16) & 0xff;
	ATA_REG(devp->addr.bus, ATA_R_DEVICE_HEAD)	   |= ATA_DH_LBA | ((first_sector >> 24) & 0xf);

	/* Issue the command */
	ATA_REG(devp->addr.bus, ATA_R_COMMAND) = ATA_CMD_WRITE_SECTORS;

	while(num_sectors--)
	{
		ata_write_data(devp->addr.bus, buf);
		buf += ATA_SECTOR_SIZE;
	}

	if(!ATA_WAIT_NBSY(devp->addr.bus))
		return DRIVER_TIMEOUT;

	/* Did an error occur? */
	if(ATA_REG(devp->addr.bus, ATA_R_STATUS) & ATA_STATUS_ERR)
	{
		/* FIXME: correctly report the error */
		return DRIVER_UNKNOWN_ERROR;
	}

	return DRIVER_OK;
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
driver_ret ata_control(void *data, ku32 function, void *in, void *out)
{
	const ata_device_t * const devp = (const ata_device_t * const) data;

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

		default:
			return DRIVER_NOT_IMPLEMENTED;
	}

	return DRIVER_OK;
}

