#ifndef DEVICE_BLOCK_PARTITION_H_INC
#define DEVICE_BLOCK_PARTITION_H_INC
/*
	partition.h: declarations of functions and types relating to the hard disc partition model

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 9th Febrary 2012.
*/

#include <device/device.h>
#include <device/devctl.h>


/* devctls */

/* types */

typedef enum
{
	PARTITION_TYPE_LINUX = 0x83
} partition_type_t;

typedef enum
{
	PARTITION_STATUS_ACTIVE = 0x00,
	PARTITION_STATUS_BOOTABLE = 0x80
} partition_status_t;


typedef struct partition_data
{
	dev_t *device;			    /* the block device hosting this partition						*/
	u32 sector_len;				/* number of bytes per sector									*/
	u32 offset;					/* offset, in sectors, of partition from the start of device	*/
	u32 len;					/* the length of this partition in sectors						*/
	partition_type_t type;
	partition_status_t status;	/* status field from partition table							*/
} partition_data_t;

block_driver_t g_partition_driver;

s32 partition_init();
s32 partition_shut_down();

s32 partition_read(dev_t *dev, ku32 offset, ku32 len, void* buf);
s32 partition_write(dev_t *dev, ku32 offset, ku32 len, const void* buf);

s32 partition_control(dev_t *dev, ku32 function, void *in, void *out);

s8 *partition_type_name(ku8 type);
s8 *partition_status_desc(ku8 status);

#endif

