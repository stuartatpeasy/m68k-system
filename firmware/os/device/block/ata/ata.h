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

struct device_driver *ata_register_driver();
driver_ret ata_init(void);
driver_ret ata_shut_down(void);

u32 ata_first_active_device_id(void);
u32 ata_next_active_device_id(ku32 device_id);

driver_ret ata_read(void *data, ku32 offset, ku32 len, void * buf);
driver_ret ata_write(void *data, ku32 offset, ku32 len, const void * buf);

driver_ret ata_control(void *data, ku32 function, void *in, void *out);

#endif

