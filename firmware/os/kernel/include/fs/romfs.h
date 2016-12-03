#ifndef KERNEL_FS_ROMFS_H_INC
#define KERNEL_FS_ROMFS_H_INC
/*
    romfs (trivial read-only file system) support

    Part of ayumos


    (c) Stuart Wallace, January 2016
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>


#define ROMFS_SUPERBLOCK


typedef struct romfs_superblock
{
    u32     magic;
    u32     len;
    time_t  cdate;
    char    label[16];
} romfs_superblock_t;

#endif
