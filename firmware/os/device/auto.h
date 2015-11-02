#ifndef DEVICE_AUTO_H_INC
#define DEVICE_AUTO_H_INC
/*
	Automatic device init and driver selection, based on hardware device ID

	Part of ayumos


	(c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/

#include <device/device.h>
#include <include/defs.h>
#include <include/types.h>


s32 dev_auto_init(ku8 hw_id, void *base_addr, ku32 irql, dev_t *parent_dev, dev_t **dev);

#endif