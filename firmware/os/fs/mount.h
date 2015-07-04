#ifndef __MOUNT_H__
#define __MOUNT_H__
/*
	Mount point functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, March 2012.
*/

#include <strings.h>
#include "include/types.h"
#include "device/device.h"

#define MOUNT_INITIAL_NUM_MOUNT_POINTS		(16)
#define MOUNT_ADDITIONAL_MOUNT_POINTS		(8)

struct mount_ent
{
	s8 *mount_point;
	device_id device;
};

s32 mount_init();
s32 mount_add(const char * const mount_point, const device_id dev);
s32 mount_remove(const char * const mount_point);
s32 mount_find(const char * const path);

#endif

