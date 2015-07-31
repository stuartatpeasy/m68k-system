/*
    FAT16 file system support

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, July 2015
*/

#include "fat.h"
#include "fs/vfs.h"

vfs_ops_t fat_ops =
{
    .mount = fat_mount;
    .umount = fat_umount;
    .fsnode_get = fat_fsnode_get;
    .fsnode_put = fat_fsnode_put;
    .locate = fat_locate;
    .stat = fat_stat;
};


int fat_mount(vfs_t *vfs)
{

    return 0;
}


int fat_umount(vfs_t *vfs)
{

    return 0;
}


int fat_fsnode_get(vfs_t *vfs, u32 node, fsnode_t * const fsn)
{

    return 0;
}


int fat_fsnode_put(vfs_t *vfs, u32 node, const fsnode_t * const fsn)
{

    return 0;
}


u32 fat_locate(vfs_t *vfs, u32 node, const char * const path)
{

}


int fat_stat(vfs_t *vfs, fs_stat_t *st)
{

    return 0;
}
