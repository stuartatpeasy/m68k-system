/*
    romfs (trivial read-only file system) support

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, January 2016
*/

#ifdef WITH_FS_ROMFS

#include <kernel/include/fs/romfs.h>
#include <kernel/include/fs/node.h>


s32 romfs_init();
s32 romfs_mount(vfs_t *vfs);
s32 romfs_unmount(vfs_t *vfs);
s32 romfs_get_root_node(vfs_t *vfs, fs_node_t **node);
s32 romfs_open_dir(vfs_t *vfs, u32 node, void **ctx);
s32 romfs_read_dir(vfs_t *vfs, void *ctx, ks8* const name, fs_node_t *node);
s32 romfs_close_dir(vfs_t *vfs, void *ctx);
s32 romfs_stat(vfs_t *vfs, fs_stat_t *st);

vfs_driver_t g_romfs_ops =
{
    .name = "romfs",
    .init = romfs_init,
    .mount = romfs_mount,
    .unmount = romfs_unmount,
    .get_root_node = romfs_get_root_node,
    .open_dir = romfs_open_dir,
    .read_dir = romfs_read_dir,
    .close_dir = romfs_close_dir,
    .stat = romfs_stat
};


s32 romfs_init()
{
    /* Nothing to do here */
    return SUCCESS;
}


s32 romfs_mount(vfs_t *vfs)
{
    UNUSED(vfs);
    return -ENOSYS;
}


s32 romfs_unmount(vfs_t *vfs)
{
    UNUSED(vfs);
    return -ENOSYS;
}


s32 romfs_get_root_node(vfs_t *vfs, fs_node_t **node)
{
    UNUSED(vfs);
    UNUSED(node);
    return -ENOSYS;
}


s32 romfs_open_dir(vfs_t *vfs, u32 node, void **ctx)
{
    UNUSED(vfs);
    UNUSED(node);
    UNUSED(ctx);
    return -ENOSYS;
}


s32 romfs_read_dir(vfs_t *vfs, void *ctx, ks8 * const name, fs_node_t *node)
{
    UNUSED(vfs);
    UNUSED(ctx);
    UNUSED(node);
    UNUSED(name);
    return -ENOSYS;
}


s32 romfs_close_dir(vfs_t *vfs, void *ctx)
{
    UNUSED(vfs);
    UNUSED(ctx);
    return -ENOSYS;
}


s32 romfs_stat(vfs_t *vfs, fs_stat_t *st)
{
    UNUSED(vfs);
    UNUSED(st);
    return -ENOSYS;
}

#endif /* WITH_FS_ROMFS */

