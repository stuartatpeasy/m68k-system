#ifndef FS_MOUNT_H_INC
#define FS_MOUNT_H_INC
/*
	Mount point functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, March 2012.
*/

#include <strings.h>
#include "fs/vfs.h"
#include "include/types.h"
#include "device/device.h"

#define MOUNT_INITIAL_NUM_MOUNT_POINTS		(16)
#define MOUNT_ADDITIONAL_MOUNT_POINTS		(8)

struct mount_ent
{
	s8 *mount_point;
	vfs_driver_t *driver;
	device_t *device;
};

struct mount_ent *g_mount_table;
u32 g_max_mounts;
u32 g_mount_end;


s32 mount_init();
s32 mount_add(const char * const mount_point, vfs_driver_t *driver, device_t *dev);
s32 mount_remove(const char * const mount_point);
s32 mount_find(const char * const path);

#endif

