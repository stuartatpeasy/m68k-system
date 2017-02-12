#ifndef KERNEL_INCLUDE_FS_PATH_H_INC
#define KERNEL_INCLUDE_FS_PATH_H_INC
/*
    Path-handling functions

    Part of ayumos


    (c) Stuart Wallace <stuartw@atom.net>, February 2017.
*/

#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/include/fs/node.h>
#include <kernel/include/fs/vfs.h>


s32 path_is_absolute(ks8 *path);
s32 path_open(const char *path, vfs_t **vfs, fs_node_t **node);

#endif
