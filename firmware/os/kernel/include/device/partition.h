#ifndef KERNEL_DEVICE_PARTITION_H_INC
#define KERNEL_DEVICE_PARTITION_H_INC
/*
	partition.h: declarations of functions and types relating to the hard disc partition model

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 9th Febrary 2012.
*/

#include <kernel/include/device/device.h>
#include <kernel/include/device/devctl.h>


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
	u32 block_size;				/* number of bytes per sector									*/
	u32 offset;					/* offset, in sectors, of partition from the start of device	*/
	partition_type_t type;
	partition_status_t status;	/* status field from partition table							*/
} partition_data_t;


s32 partition_init();
s32 partition_shut_down(dev_t *dev);

s8 *partition_type_name(ku8 type);
s8 *partition_status_desc(ku8 status);

#endif

