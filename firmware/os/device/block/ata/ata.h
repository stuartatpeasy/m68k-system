#ifndef __OS_DEVICE_BLOCK_ATA_ATA_H__
#define __OS_DEVICE_BLOCK_ATA_ATA_H__
/*
	AT Attachment (ATA) interface driver function and macro declarations

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, December 2011.
*/

#include "include/types.h"
#include "device/device.h"
#include "kutil/kutil.h"

#include <stdio.h>			/* FIXME: remove this */
#include <string.h>
#include <strings.h>

/*
	"Driver" functions
*/

struct device_driver g_ata_driver;

s32 ata_init(void);
s32 ata_shut_down(void);

u32 ata_first_active_device_id(void);
u32 ata_next_active_device_id(ku32 device_id);

s32 ata_read(void *data, ku32 offset, u32 len, void * buf);
s32 ata_write(void *data, ku32 offset, u32 len, const void * buf);

s32 ata_control(void *data, ku32 function, void *in, void *out);

s8 *ata_copy_str(s8 *dest, s8 *src, ku32 len);


#endif

