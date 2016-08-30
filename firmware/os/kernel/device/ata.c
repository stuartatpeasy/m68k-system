/*
    AT Attachment (ATA) interface driver function definitions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, December 2011.
*/

#include <kernel/device/ata.h>
#include <kernel/include/byteorder.h>
#include <kernel/include/error.h>
#include <kernel/util/kutil.h>
#include <klibc/stdio.h>            // FIXME REMOVE
#include <klibc/string.h>
#include <klibc/strings.h>


/*
    Internal functions (called by "driver" functions)
*/
s32 ata_send_command(void * const base_addr, const ata_drive_t drive, const ata_command_t cmd);

s32 ata_bus_init(dev_t *dev, const ata_drive_t drive);
s32 ata_drive_init(dev_t *dev, const ata_drive_t drive);
s32 ata_drive_do_init(dev_t *dev);
s32 ata_shut_down(dev_t *dev);

s32 ata_read(dev_t *dev, ku32 offset, u32 *len, void * buf);
s32 ata_write(dev_t *dev, ku32 offset, u32 *len, const void * buf);

void ata_read_data(vu16 * const ata_data_port, void *buf);
void ata_write_data(vu16 * const ata_data_port, const void *buf);

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
    cpu_irq_add_handler(dev->irql, dev, ata_irq);      /* Install IRQ handler */

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
    {
        kfree(dev->data);
        dev->data = NULL;
    }

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

    /* Ensure that the software reset bit is cleared, and interrupts are disabled */
    ATA_REG(base_addr, ATA_R_DEVICE_CONTROL) = ATA_DEVICE_CONTROL_NIEN;

    /* Check whether anything is plugged in */
    if((ATA_REG(base_addr, ATA_R_STATUS) & 0x7f) == 0x7f)
        return ENOMEDIUM;       /* Nothing plugged in - bus floating high */

    /* Wait for BSY to become low */
    if(!ATA_WAIT_NBSY(base_addr))
        return ETIME;

    /* Wait for DRDY to become high */
    if(!ATA_WAIT_DRDY(base_addr))
        return ENODATA;     // FIXME - better error here

    if(drive == ata_drive_master)
        ATA_REG(base_addr, ATA_R_DEVICE_HEAD) = 0;
    else if(drive == ata_drive_slave)
        ATA_REG(base_addr, ATA_R_DEVICE_HEAD) = ATA_DH_DEV;
    else
        return EINVAL;

    ATA_REG(base_addr, ATA_R_COMMAND) = ATA_CMD_IDENTIFY_DEVICE;

    /* A device is present. Wait for BSY to go low */
    if(!ATA_WAIT_NBSY(base_addr))
        return ETIME;   /* Device did not negate BSY in a timely fashion */

    /* Wait for DRDY to go high */
    if(!ATA_WAIT_DRDY(base_addr))
        return ENODATA;

    /* Did an error occur? */
    if(ATA_REG(base_addr, ATA_R_STATUS) & ATA_STATUS_ERR)
    {
        /* Was the command aborted? */
        if(ATA_REG(base_addr, ATA_R_ERROR) & ATA_ERROR_ABRT)
        {
            /* This might be a packet device. */
            if((ATA_REG(base_addr, ATA_R_SECTOR_COUNT) == 0x01) &&          /* \ this is the    */
               (ATA_REG(base_addr, ATA_R_SECTOR_NUMBER) == 0x01) &&         /* | PACKET command */
               (ATA_REG(base_addr, ATA_R_CYLINDER_LOW) == 0x14) &&          /* | feature set    */
               (ATA_REG(base_addr, ATA_R_CYLINDER_HIGH) == 0xeb) &&         /* | signature      */
               ((ATA_REG(base_addr, ATA_R_DEVICE_HEAD) & 0xef) == 0x00))    /* /                */
            {
                /* This is a packet device; these are not supported. */
                return EMEDIUMTYPE;
            }
        }

        /* Some other failure occurred; shouldn't happen during IDENTIFY DEVICE. Hey ho. */
        return EUNKNOWN;
    }

    if(ATA_REG(base_addr, ATA_R_STATUS) & ATA_STATUS_DRQ)
    {
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

    return ENODATA;
}


/*
    ata_shut_down() - shut down an ATA interface
*/
s32 ata_shut_down(dev_t *dev)
{
    if(dev->data != NULL)
        kfree(dev->data);

    return SUCCESS;
}


/*
    ata_irq() - interrupt service routine
*/
void ata_irq(ku32 irql, void *data)
{
    u8 dummy;
    UNUSED(irql);
    UNUSED(data);

    dev_t * const dev = (dev_t *) data;

    /* Clear pending interrupts */
    dummy = ATA_REG(dev->base_addr, ATA_R_STATUS);
    dummy += 0;     /* Silence "set but not used" warning */

return;

    putchar('#');

    /* TODO: implement this */
    return;
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
s32 ata_read(dev_t *dev, ku32 offset, u32 *len, void * buf)
{
    void * const base_addr = dev->base_addr;
    const ata_drive_t drive = ((ata_dev_data_t *) dev->data)->drive;
    u32 len_ = *len;

    /* Select master / slave device */
    if(drive == ata_drive_master)
        ATA_REG(base_addr, ATA_R_DEVICE_HEAD) = 0;
    else if(drive == ata_drive_slave)
        ATA_REG(base_addr, ATA_R_DEVICE_HEAD) = ATA_DH_DEV;
    else
        return ENODEV;

    if(!ATA_WAIT_NBSY(base_addr))
        return ETIME;

    if((offset + len_ < offset) || (offset + len_ > dev->len))
       return EINVAL;

    /* TODO: check that requested # sectors is allowed by the device; split into smaller reads if not. */

    ATA_REG(base_addr, ATA_R_SECTOR_COUNT)      = len_;

    ATA_REG(base_addr, ATA_R_SECTOR_NUMBER)     = offset & 0xff;
    ATA_REG(base_addr, ATA_R_CYLINDER_LOW)      = (offset >> 8) & 0xff;
    ATA_REG(base_addr, ATA_R_CYLINDER_HIGH)     = (offset >> 16) & 0xff;
    ATA_REG(base_addr, ATA_R_DEVICE_HEAD)      |= ATA_DH_LBA | ((offset >> 24) & 0xf);

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

    for(; len_--; buf += ATA_SECTOR_SIZE)
        ata_read_data(ATA_REG_DATA_ADDR(base_addr), buf);

    g_ata_stats.blocks_read += *len;
    *len -= len_;

    return SUCCESS;
}


/*
    ata_write() - write sectors to an ATA device
*/
s32 ata_write(dev_t *dev, ku32 offset, u32 *len, const void * buf)
{
    void * const base_addr = dev->base_addr;
    const ata_drive_t drive = ((ata_dev_data_t *) dev->data)->drive;
    u32 len_ = *len;

    /* Select master / slave device */
    if(drive == ata_drive_master)
        ATA_REG(base_addr, ATA_R_DEVICE_HEAD) &= ~ATA_DH_DEV;
    else if(drive == ata_drive_slave)
        ATA_REG(base_addr, ATA_R_DEVICE_HEAD) |= ATA_DH_DEV;
    else
        return ENODEV;

    if(!ATA_WAIT_NBSY(base_addr))
        return ETIME;

    if((offset + len_ < offset) || (offset + len_ > dev->len))
       return EINVAL;

    /* TODO: check that requested # sectors is allowed by the device; split into smaller reads if not. */

    ATA_REG(base_addr, ATA_R_SECTOR_COUNT)      = len_;

    ATA_REG(base_addr, ATA_R_SECTOR_NUMBER)     = offset & 0xff;
    ATA_REG(base_addr, ATA_R_CYLINDER_LOW)      = (offset >> 8) & 0xff;
    ATA_REG(base_addr, ATA_R_CYLINDER_HIGH)     = (offset >> 16) & 0xff;
    ATA_REG(base_addr, ATA_R_DEVICE_HEAD)      |= ATA_DH_LBA | ((offset >> 24) & 0xf);

    /* Issue the command */
    ATA_REG(base_addr, ATA_R_COMMAND) = ATA_CMD_WRITE_SECTORS;

    for(; len_--; buf += ATA_SECTOR_SIZE)
        ata_write_data(ATA_REG_DATA_ADDR(base_addr), buf);

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

    g_ata_stats.blocks_written += *len;
    *len -= len_;

    return SUCCESS;
}


/*
    ata_read_data() - read from the ATA data buffer
*/
void ata_read_data(vu16 * const ata_data_port, void *buf)
{
    u16 *buf_;
    u32 count;

    for(buf_ = buf, count = ATA_SECTOR_SIZE / sizeof(u16); count; count--)
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
        u16 data = *ata_data_port;
        bswap_16(data);
        *buf_++ = data;
#else
        *buf_++ = *ata_data_port;
#endif
    }
}


/*
    ata_write_data() - write to the ATA data buffer
*/
void ata_write_data(vu16 * const ata_data_port, const void *buf)
{
    ku16 *buf_;
    u32 count;

    for(buf_ = buf, count = ATA_SECTOR_SIZE / sizeof(u16); count; count--)
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
        *ata_data_port = data;
#else
        *ata_data_port = *buf_++;
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
            *((u32 *) out) = 0;     /* partitions might be bootable; the device isn't */
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
