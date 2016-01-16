/*
    romfs (trivial read-only file system) support

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, January 2016
*/

#include <kernel/fs/romfs.h>
#include <kernel/fs/vfs.h>


s32 romfs_init();
s32 romfs_mount(vfs_t *vfs);
s32 romfs_umount(vfs_t *vfs);
s32 romfs_get_root_dirent(vfs_t *vfs, vfs_dirent_t *dirent);
s32 romfs_open_dir(vfs_t *vfs, u32 node, void **ctx);
s32 romfs_read_dir(vfs_t *vfs, void *ctx, vfs_dirent_t *dirent, ks8* const name);
s32 romfs_close_dir(vfs_t *vfs, void *ctx);
s32 romfs_stat(vfs_t *vfs, fs_stat_t *st);

vfs_driver_t g_romfs_ops =
{
    .name = "romfs",
    .init = romfs_init,
    .mount = romfs_mount,
    .umount = romfs_umount,
    .get_root_dirent = romfs_get_root_dirent,
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
    return ENOSYS;
}


s32 romfs_umount(vfs_t *vfs)
{
    UNUSED(vfs);
    return ENOSYS;
}


s32 romfs_get_root_dirent(vfs_t *vfs, vfs_dirent_t *dirent)
{
    UNUSED(vfs);
    UNUSED(dirent);
    return ENOSYS;
}


s32 romfs_open_dir(vfs_t *vfs, u32 node, void **ctx)
{
    UNUSED(vfs);
    UNUSED(node);
    UNUSED(ctx);
    return ENOSYS;
}


s32 romfs_read_dir(vfs_t *vfs, void *ctx, vfs_dirent_t *dirent, ks8 * const name)
{
    UNUSED(vfs);
    UNUSED(ctx);
    UNUSED(dirent);
    UNUSED(name);
    return ENOSYS;
}


s32 romfs_close_dir(vfs_t *vfs, void *ctx)
{
    UNUSED(vfs);
    UNUSED(ctx);
    return ENOSYS;
}


s32 romfs_stat(vfs_t *vfs, fs_stat_t *st)
{
    UNUSED(vfs);
    UNUSED(st);
    return ENOSYS;
}
