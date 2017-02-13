#ifndef KERNEL_INCLUDE_FS_ROMFS_H_INC
#define KERNEL_INCLUDE_FS_ROMFS_H_INC
/*
    romfs (trivial read-only file system) support

    Part of ayumos


    (c) Stuart Wallace, January 2016
*/

#ifdef WITH_FS_ROMFS

#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/include/fs/vfs.h>


#define ROMFS_SUPERBLOCK


typedef struct romfs_superblock
{
    u32     magic;
    u32     len;
    time_t  cdate;
    char    label[16];
} romfs_superblock_t;

vfs_driver_t g_romfs_ops;

#endif /* WITH_FS_ROMFS */
#endif
