#ifndef KERNEL_FS_MOUNT_H_INC
#define KERNEL_FS_MOUNT_H_INC
/*
	Mount point functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, March 2012.
*/

#include <kernel/include/device/device.h>
#include <kernel/include/fs/vfs.h>
#include <kernel/include/types.h>
#include <klibc/include/strings.h>


struct mount_ent;
typedef struct mount_ent mount_ent_t;

struct mount_ent
{
	s8 *            mount_point;
	u32             mount_point_len;
	vfs_t *         vfs;
	mount_ent_t *   next;
};


s32 mount_init();
s32 mount_add(const char * const mount_point, vfs_driver_t *driver, dev_t *dev);
s32 mount_remove(const char * const mount_point);
vfs_t *mount_find(const char * const path, const char **rel);

#endif

