#ifndef DEVICE_BLOCK_ATA_ATA_H_INC
#define DEVICE_BLOCK_ATA_ATA_H_INC
/*
	AT Attachment (ATA) interface driver function and macro declarations

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, December 2011.
*/

#include "include/byteorder.h"
#include "include/types.h"
#include "device/block/block.h"
#include "device/device.h"
#include "kutil/kutil.h"

#include <stdio.h>			/* FIXME: remove this */
#include <string.h>
#include <strings.h>

/*
	"Driver" functions
*/

blockdev_stats_t g_ata_stats;

struct device_driver g_ata_driver;

s32 ata_init(void);
s32 ata_shut_down(void);

u32 ata_first_active_device_id(void);
u32 ata_next_active_device_id(ku32 device_id);

s32 ata_read(void *data, ku32 offset, u32 len, void * buf);
s32 ata_write(void *data, ku32 offset, u32 len, const void * buf);

s32 ata_control(void *data, ku32 function, void *in, void *out);


#endif

