/*
	Virtual file system abstraction

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, July 2012.
*/

#include <kernel/include/device/devctl.h>
#include <kernel/include/device/device.h>
#include <kernel/include/device/nvram.h>
#include <kernel/include/fs/vfs.h>
#include <kernel/include/fs/mount.h>


/*
    #include driver registration function declarations here
*/
#include <kernel/fs/fat/fat.h>
#include <kernel/fs/ext2/ext2.h>

vfs_driver_t * g_fs_drivers[] =
{
#ifdef WITH_FS_FAT
    &g_fat_ops,
#endif
#ifdef WITH_FS_EXT2
    &g_ext2_ops,
#endif
};


s32 vfs_init()
{
	nvram_bpb_t bpb;
	s32 ret;
	dev_t *dev;
	vfs_driver_t ** ppdrv;

	/* Init file system drivers */
    FOR_EACH(ppdrv, g_fs_drivers)
    {
        vfs_driver_t * const pdrv = *ppdrv;

        ret = pdrv->init();
        if(ret == SUCCESS)
        {
            /*
                If the driver has chosen not to expose any function, point it at the default version
                of that function.  The default version simply returns ENOSYS.
            */
            if(NULL == pdrv->mount)           pdrv->mount           = vfs_default_mount;
            if(NULL == pdrv->umount)          pdrv->umount          = vfs_default_umount;
            if(NULL == pdrv->get_root_dirent) pdrv->get_root_dirent = vfs_default_get_root_dirent;
            if(NULL == pdrv->open_dir)        pdrv->open_dir        = vfs_default_open_dir;
            if(NULL == pdrv->read_dir)        pdrv->read_dir        = vfs_default_read_dir;
            if(NULL == pdrv->close_dir)       pdrv->close_dir       = vfs_default_close_dir;
            if(NULL == pdrv->stat)            pdrv->stat            = vfs_default_stat;

            printf("vfs: initialised '%s' fs driver\n", pdrv->name);
        }
        else
        {
            /* TODO handle this - make the fs driver unavailable */
            printf("vfs: failed to initialise '%s' fs driver: %s\n", pdrv->name, kstrerror(ret));
        }
    }

    ret = mount_init();
    if(ret != SUCCESS)
        return ret;

	/* Find rootfs device */
    ret = nvram_bpb_read(&bpb);
	if(ret != SUCCESS)
    {
        printf("vfs: failed to read rootfs from BPB: %s\n", kstrerror(ret));
        return ret;
    }

    dev = dev_find(bpb.rootfs);
    if(dev == NULL)
    {
        printf("vfs: rootfs partition '%s' not found\n", bpb.rootfs);
        return ENODEV;
    }

    if((dev->type != DEV_TYPE_BLOCK) || (dev->subtype != DEV_SUBTYPE_PARTITION))
    {
        printf("vfs: rootfs '%s' is not a partition device\n", bpb.rootfs);
        return ENODEV;
    }

    /* Find FS driver corresponding to bpb.fstype */
    vfs_driver_t *driver = vfs_get_driver_by_name(bpb.fstype);
    if(!driver)
    {
        printf("vfs: unknown filesystem type '%s' specified\n", bpb.fstype);
        return EINVAL;
    }

    /* Found fs driver */
    printf("vfs: rootfs: %s (%s)\n", bpb.rootfs, bpb.fstype);

    return mount_add("/", driver, dev);
}


/* === Default handlers for the functions in vfs_driver_t.  These all return ENOSYS. === */
s32 vfs_default_mount(vfs_t *vfs)
{
    UNUSED(vfs);

    return ENOSYS;
}


s32 vfs_default_umount(vfs_t *vfs)
{
    UNUSED(vfs);

    return ENOSYS;
}


s32 vfs_default_get_root_dirent(vfs_t *vfs, vfs_dirent_t *dirent)
{
    UNUSED(vfs);
    UNUSED(dirent);

    return ENOSYS;
}


s32 vfs_default_open_dir(vfs_t *vfs, u32 node, void **ctx)
{
    UNUSED(vfs);
    UNUSED(node);
    UNUSED(ctx);

    return ENOSYS;
}


s32 vfs_default_read_dir(vfs_t *vfs, void *ctx, vfs_dirent_t *dirent, ks8 * const name)
{
    UNUSED(vfs);
    UNUSED(ctx);
    UNUSED(dirent);
    UNUSED(name);

    return ENOSYS;
}


s32 vfs_default_close_dir(vfs_t *vfs, void *ctx)
{
    UNUSED(vfs);
    UNUSED(ctx);

    return ENOSYS;
}


s32 vfs_default_stat(vfs_t *vfs, fs_stat_t *st)
{
    UNUSED(vfs);
    UNUSED(st);

    return ENOSYS;
}
/* === END default handlers for functions in vfs_driver_t === */


/*
    vfs_get_driver_by_name() - look up a VFS driver by name.
*/
vfs_driver_t *vfs_get_driver_by_name(ks8 * const name)
{
    vfs_driver_t **ppdrv;

    FOR_EACH(ppdrv, g_fs_drivers)
    {
        vfs_driver_t * const pdrv = *ppdrv;

        if(!strcmp(pdrv->name, name))
            return pdrv;
    }

    return NULL;
}


/*
    vfs_open_dir() - "open" a directory, i.e. find the device containing the path, verify the path,
    ensure that the path represents a directory, and prepare to iterate over directory entries.
*/
s32 vfs_open_dir(ks8 *path, vfs_dir_ctx_t *ctx)
{
    vfs_dirent_t dirent;
    vfs_t *vfs;
    s32 ret;

    ret = vfs_lookup(path, &dirent);
    if(ret != SUCCESS)
        return ret;

    if(dirent.type != FSNODE_TYPE_DIR)
        return ENOTDIR;

    vfs = dirent.vfs;
    ret = vfs->driver->open_dir(vfs, dirent.first_node, &(ctx->ctx));
    if(ret != SUCCESS)
        return ret;

    ctx->vfs = vfs;

    return SUCCESS;
}


/*
    vfs_read_dir() - read the next item from a directory "opened" by vfs_open_dir().
*/
s32 vfs_read_dir(vfs_dir_ctx_t *ctx, vfs_dirent_t *dirent, ks8 * const name)
{
    return ctx->vfs->driver->read_dir(ctx->vfs, ctx->ctx, dirent, name);
}


/*
    vfs_close_dir() - clean up after iterating over a directory using vfs_open_dir()/vfs_read_dir().
*/
s32 vfs_close_dir(vfs_dir_ctx_t *ctx)
{
    return ctx->vfs->driver->close_dir(ctx->vfs, ctx->ctx);
}


/*
    vfs_lookup() - look up a path and populate a vfs_dirent_t with the contents.
*/
s32 vfs_lookup(ks8 * path, vfs_dirent_t *ent)
{
    vfs_t *vfs;
    const char *rel;
    s8 * path_component;
    void *ctx;
    u32 i, node;
    s32 ret;

    if(strlen(path) > NAME_MAX_LEN)
        return ENAMETOOLONG;

    vfs = mount_find(path, &rel);
    if(vfs == NULL)
        return ENOENT;      /* Should only happen if no root fs is mounted */

    node = vfs->root_node;

    for(; *rel == DIR_SEPARATOR; ++rel)
        ;                           /* Skip over empty path components */

    if(rel[0] == '\0')
        return vfs->driver->get_root_dirent(vfs, ent);  /* Special case: root directory */

    path_component = (s8 *) kmalloc(NAME_MAX_LEN + 1);
    if(!path_component)
        return ENOMEM;

    do
    {
        for(; *rel == DIR_SEPARATOR; ++rel)
            ;                       /* Skip over empty path components */

        for(i = 0; (*rel != DIR_SEPARATOR) && (*rel != '\0'); ++i)
            path_component[i] = *rel++;

        if(!i)
            continue;
        else if(i == NAME_MAX_LEN)
        {
            kfree(path_component);
            return ENAMETOOLONG;
        }

        path_component[i] = '\0';

        ret = vfs->driver->open_dir(vfs, node, &ctx);
        if(ret != SUCCESS)
        {
            kfree(path_component);
            return ret;
        }

        ret = vfs->driver->read_dir(vfs, ctx, ent, path_component);

        vfs->driver->close_dir(vfs, ctx);
        if(ret != SUCCESS)
        {
            kfree(path_component);
            return ret;
        }

        node = ent->first_node;

        /* Check that the located component is a directory or a file, as appropriate */
        if((*rel == DIR_SEPARATOR) && (ent->type == FSNODE_TYPE_FILE))
        {
            kfree(path_component);
            return ENOTDIR;         /* Looking for a directory but found a file */
        }

        if((*rel == '\0') && (ent->type == FSNODE_TYPE_DIR))
        {
            kfree(path_component);
            return EISDIR;          /* Looking for a file but found a directory */
        }
    } while(*rel++ != '\0');

    kfree(path_component);

    return SUCCESS;
}


/*
    vfs_dirent_perm_str() build in str a ten-character "permission string", e.g. "drwxr-x---" from
    the supplied dirent.  str must point to a buffer of at least 10 characters.

    TODO: this probably shouldn't be here; put it somewhere else.
*/
s8 *vfs_dirent_perm_str(const vfs_dirent_t * const dirent, s8 *str)
{
    const file_perm_t perm = dirent->permissions;
    const fsnode_type_t type = dirent->type;

    str[0] = (type == FSNODE_TYPE_DIR) ? 'd' : '-';
    str[1] = (perm & VFS_PERM_UR) ? 'r' : '-';
    str[2] = (perm & VFS_PERM_UW) ? 'w' : '-';
    str[3] = (perm & VFS_PERM_UX) ?
                ((perm & VFS_PERM_UT) ? 's' : 'x') :
                ((perm & VFS_PERM_UT) ? 'S' : '-');
    str[4] = (perm & VFS_PERM_GR) ? 'r' : '-';
    str[5] = (perm & VFS_PERM_GW) ? 'w' : '-';
    str[6] = (perm & VFS_PERM_GW) ?
                ((perm & VFS_PERM_GT) ? 's' : 'x') :
                ((perm & VFS_PERM_GT) ? 'S' : '-');
    str[7] = (perm & VFS_PERM_OR) ? 'r' : '-';
    str[8] = (perm & VFS_PERM_OW) ? 'w' : '-';
    str[9] = (perm & VFS_PERM_OX) ?
                ((perm & VFS_PERM_OT) ? 's' : 'x') :
                ((perm & VFS_PERM_OT) ? 'S' : '-');

    return str;
}
