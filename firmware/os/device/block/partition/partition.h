#ifndef __OS_DEVICE_BLOCK_PARTITION_H__
#define __OS_DEVICE_BLOCK_PARTITION_H__
/*
	partition.h: declarations of functions and types relating to the hard disc partition model

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 9th Febrary 2012.
*/

#include "device/device.h"
#include "device/devctl.h"
#include "mbr.h"

#include <strings.h>

#define MAX_PARTITIONS	(16)		/* FIXME: remove this; use dynamic allocation */

/* devctls */

/* types */

typedef enum
{
	PARTITION_TYPE_LINUX = 0x83
} partition_type;

typedef enum
{
	PARTITION_STATUS_ACTIVE = 0x00,
	PARTITION_STATUS_BOOTABLE = 0x80
} partition_status;


struct partition_data
{
	device_t *device;			/* the block device hosting this partition						*/
	u32 sector_len;				/* number of bytes per sector									*/
	u32 offset;					/* offset, in sectors, of partition from the start of device	*/
	u32 len;					/* the length of this partition in sectors						*/
	partition_type type;
	partition_status status;	/* status field from partition table							*/
};

struct device_driver g_partition_driver;

s32 partition_init();
s32 partition_shut_down();

s32 partition_read(void *data, ku32 offset, ku32 len, void* buf);
s32 partition_write(void *data, ku32 offset, ku32 len, const void* buf);

s32 partition_control(void *data, ku32 function, void *in, void *out);

s8 *partition_type_name(ku8 type);
s8 *partition_status_desc(ku8 status);

#endif

