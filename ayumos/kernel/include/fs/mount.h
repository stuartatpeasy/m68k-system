#ifndef KERNEL_INCLUDE_FS_MOUNT_H_INC
#define KERNEL_INCLUDE_FS_MOUNT_H_INC
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
    vfs_t *         host_vfs;
    fs_node_t *     host_node;
    vfs_t *         inner_vfs;
    mount_ent_t *   next;
};


s32 mount_init();
s32 mount_add(vfs_t * const host_vfs, fs_node_t * const host_node, vfs_driver_t * const driver,
              dev_t * const dev);
s32 mount_remove(const vfs_t * const host_vfs, const fs_node_t * const host_node,
                 const dev_t * const dev);
s32 mount_find(const vfs_t * const vfs_outer, const fs_node_t * const node_outer, vfs_t **vfs_inner,
               fs_node_t **node_inner);


#endif

