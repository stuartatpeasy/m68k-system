#ifndef KERNEL_FS_FAT_FAT_H_INC
#define KERNEL_FS_FAT_FAT_H_INC
/*
    FAT16 file system support

    Part of the ayumos


    (c) Stuart Wallace, July 2015
*/

#ifdef WITH_FS_FAT

#include <kernel/include/fs/vfs.h>


vfs_driver_t g_fat_ops;

#endif /* WITH_FS_FAT */
#endif
