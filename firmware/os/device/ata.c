/*
	AT Attachment (ATA) interface driver function definitions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, December 2011.
*/

#include <device/ata.h>
#include <kutil/kutil.h>
#include <include/byteorder.h>
#include <include/error.h>

#include <string.h>
#include <strings.h>


/*
	Internal functions (called by "driver" functions)
*/
s32 ata_send_command(void * const base_addr, const ata_drive_t drive, const ata_command_t cmd);

s32 ata_bus_init(dev_t *dev, const ata_drive_t drive);
s32 ata_drive_init(dev_t *dev, const ata_drive_t drive);
s32 ata_drive_do_init(dev_t *dev);
s32 ata_shut_down(dev_t *dev);

s32 ata_read(dev_t *dev, ku32 offset, u32 len, void * buf);
s32 ata_write(dev_t *dev, ku32 offset, u32 len, const void * buf);

void ata_read_data(const void * const base_addr, void *buf);
void ata_write_data(void * const base_addr, const void *buf);

s32 ata_control(dev_t *dev, ku32 function, const void *in, void *out);
s32 ata_drive_control(dev_t *dev, const devctl_fn_t fn, const void *in, void *out);


blockdev_stats_t g_ata_stats;


/*
    ata_init() - initialise an ATA interface
*/
s32 ata_init(dev_t *dev)
{
    /* Disable ATA interrupts, and ensure that SRST is cleared for the bus */
    ATA_REG(dev->base_addr, ATA_R_DEVICE_CONTROL) = ATA_DEVICE_CONTROL_NIEN;

    /* TODO: re-enable ATA interrupts... */

    return SUCCESS;
}


/*
    ata_master_init() - initialise the master device on the ATA bus
*/
s32 ata_master_init(dev_t * dev)
{
    return ata_drive_init(dev, ata_drive_master);
}


/*
    ata_channel_0_slave_init() - initialise the slave device on the ATA bus
*/
s32 ata_slave_init(dev_t * dev)
{
    return ata_drive_init(dev, ata_drive_slave);
}


/*
    ata_drive_init() - set up control structures for a drive, try to init the drive
*/
s32 ata_drive_init(dev_t *dev, const ata_drive_t drive)
{
    ata_dev_data_t *dev_data;
    s32 ret;

    dev_data = (ata_dev_data_t *) CHECKED_KCALLOC(1, sizeof(ata_dev_data_t));

    dev->read       = ata_read;
    dev->write      = ata_write;
    dev->control    = ata_drive_control;
    dev->shut_down  = ata_shut_down;
    dev->block_size = ATA_SECTOR_SIZE;

    dev_data->drive = drive;
    dev->data = dev_data;

    ret = ata_drive_do_init(dev);

    if(ret != SUCCESS)
        kfree(dev_data);

    return ret;
}


/*
    ata_drive_do_init() - initialise a drive on the ATA bus
*/
s32 ata_drive_do_init(dev_t *dev)
{
    void * const base_addr = dev->base_addr;
    ata_dev_data_t * const dev_data = (ata_dev_data_t *) dev->data;
    const ata_drive_t drive = dev_data->drive;
    u16 *p;
    ata_identify_device_ret_t id;

    if(drive == ata_drive_master)
        ATA_REG(base_addr, ATA_R_DEVICE_HEAD) &= ~ATA_DH_DEV;
    else if(drive == ata_drive_slave)
        ATA_REG(base_addr, ATA_R_DEVICE_HEAD) |= ATA_DH_DEV;
    else
        return EINVAL;

    /* Wait for BSY to become low */
    if(!ATA_WAIT_NBSY(base_addr))
        return ETIME;

    ATA_REG(base_addr, ATA_R_COMMAND) = ATA_CMD_IDENTIFY_DEVICE;

        /* Wait for BSY to go high */
    if(!ATA_WAIT_BSY(base_addr))
        return ENOMEDIUM;   /* No response from device; presumably nothing plugged in to the port */

    /* A device is present. Wait for BSY to go low */
    if(!ATA_WAIT_NBSY(base_addr))
        return ETIME;	/* Device did not negate BSY in a timely fashion */

    /* Did an error occur? */
    if(ATA_REG(base_addr, ATA_R_STATUS) & ATA_STATUS_ERR)
    {
        /* Was the command aborted? */
        if(ATA_REG(base_addr, ATA_R_ERROR) & ATA_ERROR_ABRT)
        {
            /* This might be a packet device. */
            if((ATA_REG(base_addr, ATA_R_SECTOR_COUNT) == 0x01) &&		    /* \ this is the 	*/
               (ATA_REG(base_addr, ATA_R_SECTOR_NUMBER) == 0x01) &&			/* | PACKET command	*/
               (ATA_REG(base_addr, ATA_R_CYLINDER_LOW) == 0x14) &&		    /* | feature set 	*/
               (ATA_REG(base_addr, ATA_R_CYLINDER_HIGH) == 0xeb) &&			/* | signature	 	*/
               ((ATA_REG(base_addr, ATA_R_DEVICE_HEAD) & 0xef) == 0x00))	/* /			 	*/
            {
                /* This is a packet device; these are not supported. */
                return EMEDIUMTYPE;
            }
        }

        /* Some other failure occurred; shouldn't happen during IDENTIFY DEVICE. Hey ho. */
        return EUNKNOWN;
    }

    ata_read_data(base_addr, &id);

    /*
        For some reason, the strings returned in the IDENTIFY DEVICE structure are big-
        endian.  All of the rest of the data in the response is little-endian.  Why not?
    */
    for(p = (u16 *) &id.model_number;
        p < (u16 *) (&id.model_number[sizeof(id.model_number)]); ++p)
        *p = LE2N16(*p);

    for(p = (u16 *) &id.serial_number;
        p < (u16 *) (&id.serial_number[sizeof(id.serial_number)]); ++p)
        *p = LE2N16(*p);

    for(p = (u16 *) &id.firmware_revision;
        p < (u16 *) (&id.firmware_revision[sizeof(id.firmware_revision)]); ++p)
        *p = LE2N16(*p);

    strn_trim_cpy(dev_data->model, id.model_number, sizeof(id.model_number));
    strn_trim_cpy(dev_data->serial, id.serial_number, sizeof(id.serial_number));
    strn_trim_cpy(dev_data->firmware, id.firmware_revision, sizeof(id.firmware_revision));

    dev->len = LE2N16(id.log_cyls) * LE2N16(id.log_heads) * LE2N16(id.log_sects_per_log_track);

    return SUCCESS;
}


/*
    ata_shut_down() - shut down an ATA interface
*/
s32 ata_shut_down(dev_t *dev)
{
    kfree(dev->data);
	return SUCCESS;
}


/*
	ata_send_command() - Send ATA command
*/
s32 ata_send_command(void * const base_addr, const ata_drive_t drive, const ata_command_t cmd)
{
	/* Select master / slave device */
	if(drive == ata_drive_master)
		ATA_REG(base_addr, ATA_R_DEVICE_HEAD) &= ~ATA_DH_DEV;
	else if(drive == ata_drive_slave)
		ATA_REG(base_addr, ATA_R_DEVICE_HEAD) |= ATA_DH_DEV;
	else
		return ENODEV;

	/* Wait for the device to become ready */
	if(!ATA_WAIT_NBSY(base_addr))
		return ETIME;

	ATA_REG(base_addr, ATA_R_COMMAND) = cmd;

	/* Wait for the command to complete */
	if(!ATA_WAIT_NBSY(base_addr))
		return ETIME;

	/* Did an error occur? */
	if(ATA_REG(base_addr, ATA_R_STATUS) & ATA_STATUS_ERR)
	{
		/* Note: it is up to the caller to determine which error occurred */
		return EDEVOPFAILED;
	}

	return SUCCESS;
}


/*
    ata_read() - read sectors from an ATA device
*/
s32 ata_read(dev_t *dev, ku32 offset, u32 len, void * buf)
{
    void * const base_addr = dev->base_addr;
    const ata_drive_t drive = ((ata_dev_data_t *) dev->data)->drive;

	/* Select master / slave device */
	if(drive == ata_drive_master)
		ATA_REG(base_addr, ATA_R_DEVICE_HEAD) &= ~ATA_DH_DEV;
	else if(drive == ata_drive_slave)
		ATA_REG(base_addr, ATA_R_DEVICE_HEAD) |= ATA_DH_DEV;
	else
		return ENODEV;

	if(!ATA_WAIT_NBSY(base_addr))
		return ETIME;

	if((offset + len < offset) || (offset + len > dev->len))
	   return EINVAL;

	/* TODO: check that requested # sectors is allowed by the device; split into smaller reads if not. */

	ATA_REG(base_addr, ATA_R_SECTOR_COUNT)		= len;

	ATA_REG(base_addr, ATA_R_SECTOR_NUMBER)		= offset & 0xff;
	ATA_REG(base_addr, ATA_R_CYLINDER_LOW)		= (offset >> 8) & 0xff;
	ATA_REG(base_addr, ATA_R_CYLINDER_HIGH)		= (offset >> 16) & 0xff;
	ATA_REG(base_addr, ATA_R_DEVICE_HEAD)	   |= ATA_DH_LBA | ((offset >> 24) & 0xf);

	/* Issue the command */
	ATA_REG(base_addr, ATA_R_COMMAND) = ATA_CMD_READ_SECTORS;

	if(!ATA_WAIT_NBSY(base_addr))
		return ETIME;

	if(ATA_REG(base_addr, ATA_R_STATUS) & ATA_STATUS_ERR)
	{
		ku8 err = ATA_REG(base_addr, ATA_R_ERROR);
		if((err & ATA_ERROR_ABRT) || (err & ATA_ERROR_IDNF))
			return EINVAL;
		else if(err & ATA_ERROR_UNC)
			return EDATA;
		else if((err & ATA_ERROR_MC) || (err & ATA_ERROR_MCR))
			return EMEDIACHANGED;
		else if(err & ATA_ERROR_NM)
			return ENOMEDIUM;
		else
			return EUNKNOWN;
	}

	for(; len--; buf += ATA_SECTOR_SIZE)
		ata_read_data(base_addr, buf);

	g_ata_stats.blocks_read += len;

	return SUCCESS;
}


/*
    ata_write() - write sectors to an ATA device
*/
s32 ata_write(dev_t *dev, ku32 offset, u32 len, const void * buf)
{
    void * const base_addr = dev->base_addr;
    const ata_drive_t drive = ((ata_dev_data_t *) dev->data)->drive;

	/* Select master / slave device */
	if(drive == ata_drive_master)
		ATA_REG(base_addr, ATA_R_DEVICE_HEAD) &= ~ATA_DH_DEV;
	else if(drive == ata_drive_slave)
		ATA_REG(base_addr, ATA_R_DEVICE_HEAD) |= ATA_DH_DEV;
	else
		return ENODEV;

	if(!ATA_WAIT_NBSY(base_addr))
		return ETIME;

	if((offset + len < offset) || (offset + len > dev->len))
	   return EINVAL;

	/* TODO: check that requested # sectors is allowed by the device; split into smaller reads if not. */

	ATA_REG(base_addr, ATA_R_SECTOR_COUNT)	 	= len;

	ATA_REG(base_addr, ATA_R_SECTOR_NUMBER)	 	= offset & 0xff;
	ATA_REG(base_addr, ATA_R_CYLINDER_LOW)	 	= (offset >> 8) & 0xff;
	ATA_REG(base_addr, ATA_R_CYLINDER_HIGH)		= (offset >> 16) & 0xff;
	ATA_REG(base_addr, ATA_R_DEVICE_HEAD)      |= ATA_DH_LBA | ((offset >> 24) & 0xf);

	/* Issue the command */
	ATA_REG(base_addr, ATA_R_COMMAND) = ATA_CMD_WRITE_SECTORS;

	for(; len--; buf += ATA_SECTOR_SIZE)
		ata_write_data(base_addr, buf);

	if(!ATA_WAIT_NBSY(base_addr))
		return ETIME;

	/* Did an error occur? */
	if(ATA_REG(base_addr, ATA_R_STATUS) & ATA_STATUS_ERR)
	{
		ku8 err = ATA_REG(base_addr, ATA_R_ERROR);
		if((err & ATA_ERROR_ABRT) || (err & ATA_ERROR_IDNF))
			return EINVAL;
		else if(err & ATA_ERROR_WP)
			return EROFS;
		else if(err & ATA_ERROR_UNC)
			return EDATA;
		else if((err & ATA_ERROR_MC) || (err & ATA_ERROR_MCR))
			return EMEDIACHANGED;
		else if(err & ATA_ERROR_NM)
			return ENOMEDIUM;
		else
			return EUNKNOWN;
	}

	g_ata_stats.blocks_written += len;

	return SUCCESS;
}


/*
	ata_read_data() - read from the ATA data buffer
*/
void ata_read_data(const void * const base_addr, void *buf)
{
	u16 *buf_ = buf;
	u32 count = (ATA_SECTOR_SIZE / sizeof(u16));

	for(; count--;)
    {
#ifdef BUG_ATA_BYTE_SWAP
        /*
            The lambda rev0 pcb has its ATA data bus connected to the CPU data bus as:

                ATA_D[15..8] <=> CPU_D[15..8]
                ATA_D[7..0]  <=> CPU_D[7..0]

            This seems correct but creates interop problems with volumes created on little-endian
            hosts.  The desired electrical arrangement is:

                ATA_D[15..8] <=> CPU_D[7..0]
                ATA_D[7..0]  <=> CPU_D[15..8]

            ...i.e. byte-swapping.  This results in single bytes, being stored in memory in the
            "correct" order when read from / written to little-endian file systems.  Note that
            16-/32-bit (or larger) numerics need to be converted to target endianness under this
            arrangement.

           If the BUG_ATA_BYTE_SWAP constant is defined, we therefore swap the bytes in each 16-bit
           datum read/written in order to work round the incorrect board design.
        */
        u16 data = ATA_REG_DATA(base_addr);
        bswap_16(data);
        *buf_++ = data;
#else
		*buf_++ = ATA_REG_DATA(base_addr);
#endif
    }
}


/*
	ata_write_data() - write to the ATA data buffer
*/
void ata_write_data(void * const base_addr, const void *buf)
{
	const u16 *buf_ = buf;
	u32 count = (ATA_SECTOR_SIZE / sizeof(u16));

	for(; count--;)
    {
#ifdef BUG_ATA_BYTE_SWAP
        /*
            The lambda rev0 pcb has its ATA data bus connected to the CPU data bus as:

                ATA_D[15..8] <=> CPU_D[15..8]
                ATA_D[7..0]  <=> CPU_D[7..0]

            This seems correct but creates interop problems with volumes created on little-endian
            hosts.  The desired electrical arrangement is:

                ATA_D[15..8] <=> CPU_D[7..0]
                ATA_D[7..0]  <=> CPU_D[15..8]

            ...i.e. byte-swapping.  This results in single bytes, being stored in memory in the
            "correct" order when read from / written to little-endian file systems.  Note that
            16-/32-bit (or larger) numerics need to be converted to target endianness under this
            arrangement.

           If the BUG_ATA_BYTE_SWAP constant is defined, we therefore swap the bytes in each 16-bit
           datum read/written in order to work round the incorrect board design.
        */
        u16 data = *buf_++;
        bswap_16(data);
		ATA_REG_DATA(base_addr) = data;
#else
		ATA_REG_DATA(base_addr) = *buf_++;
#endif
    }
}


/*
    ata_control() - ATA devctl implementation for the interface device
*/

s32 ata_control(dev_t *dev, ku32 function, const void *in, void *out)
{
    UNUSED(dev);
    UNUSED(function);
    UNUSED(in);
    UNUSED(out);

    return ENOSYS;
}


/*
	ata_drive_control() - ATA devctl implementation for attached drives
*/
s32 ata_drive_control(dev_t *dev, const devctl_fn_t fn, const void *in, void *out)
{
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
			*((u32 *) out) = 0;		/* partitions might be bootable; the device isn't */
			break;

        case dc_get_model:
            *((s8 **) out) = ((ata_dev_data_t *) dev->data)->model;
            break;

        case dc_get_serial:
            *((s8 **) out) = ((ata_dev_data_t *) dev->data)->serial;
            break;

        case dc_get_firmware_ver:
            *((s8 **) out) = ((ata_dev_data_t *) dev->data)->firmware;
            break;

		default:
			return ENOSYS;
	}

	return SUCCESS;
}
