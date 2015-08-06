#ifndef FS_FAT_FAT_H_INC
#define FS_FAT_FAT_H_INC
/*
    FAT16 file system support

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, July 2015
*/

#include "include/defs.h"
#include "include/types.h"
#include "fs/fat/fat-defs.h"
#include "fs/vfs.h"

vfs_ops_t fat_ops;

s32 fat_mount(vfs_t *vfs);
s32 fat_umount(vfs_t *vfs);
s32 fat_fsnode_get(vfs_t *vfs, u32 node, fsnode_t * const fsn);
s32 fat_fsnode_put(vfs_t *vfs, u32 node, const fsnode_t * const fsn);
u32 fat_locate(vfs_t *vfs, u32 node, const char * const path);
s32 fat_stat(vfs_t *vfs, fs_stat_t *st);

#endif
