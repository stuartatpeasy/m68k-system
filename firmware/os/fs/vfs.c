/*
	Virtual file system abstraction

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, July 2012.
*/

#include "fs/vfs.h"
#include "fs/mount.h"
#include "device/bbram.h"
#include "device/device.h"
#include "device/devctl.h"


vfs_t *g_filesystems[MAX_FILESYSTEMS];

/*
    #include driver registration function declarations here
*/
#include "fs/fat/fat.h"
#include "fs/ext2/ext2.h"

vfs_driver_t *g_fs_drivers[] =
{
    &g_fat_ops
};


dev_t *find_boot_device()
{
	u32 i;

	for(i = 0; i < MAX_DEVICES; ++i)
	{
        if(g_devices[i] != NULL)
        {
            dev_t * const dev = g_devices[i];
            if(dev->type == DEV_TYPE_BLOCK)
            {
                u32 bootable = 0;

                if(device_control(dev, DEVCTL_BOOTABLE, NULL, &bootable) == SUCCESS)
                {
                    if(bootable)
                        return dev;
                }
            }
        }
	}

	return NULL;
}


s32 vfs_init()
{
	bbram_param_block_t bpb;
	s32 ret, i;

	/* Init file system drivers */
	for(i = 0; i < ARRAY_COUNT(g_fs_drivers); ++i)
    {
        vfs_driver_t * const drv = g_fs_drivers[i];

        printf("vfs: initialising '%s' fs driver: ", drv->name);
        if(drv->init() == SUCCESS)
            puts("OK");
        else
            puts("failed");     /* TODO handle this - make the fs driver unavailable */

        /*
            If the driver has chosen not to expose any function, point it at the default version
            of that function.  The default version simply returns ENOSYS.
        */
        if(NULL == drv->mount)           drv->mount           = vfs_default_mount;
        if(NULL == drv->umount)          drv->umount          = vfs_default_umount;
        if(NULL == drv->get_root_dirent) drv->get_root_dirent = vfs_default_get_root_dirent;
        if(NULL == drv->open_dir)        drv->open_dir        = vfs_default_open_dir;
        if(NULL == drv->read_dir)        drv->read_dir        = vfs_default_read_dir;
        if(NULL == drv->close_dir)       drv->close_dir       = vfs_default_close_dir;
        if(NULL == drv->stat)            drv->stat            = vfs_default_stat;
    }

    ret = mount_init();
    if(ret != SUCCESS)
        return ret;

#if 0
/* FIXME - reinstate this code */
	/* Find rootfs device */
	ret = bbram_param_block_read(&bpb);
	if(ret == SUCCESS)
    {
        /* Iterate over partition devices, looking for one whose name matches the BPB rootfs */
        for(i = 0; i < MAX_DEVICES; ++i)
        {
            if(g_devices[i] != NULL)
            {
                dev_t * const dev = g_devices[i];
                if((dev->type == DEV_TYPE_BLOCK) && (dev->subtype == DEV_SUBTYPE_PARTITION) &&
                   !strcmp(dev->name, bpb.rootfs))
                {
                    /* Find FS driver corresponding to bpb.fstype */
                    vfs_driver_t *driver = vfs_get_driver_by_name(bpb.fstype);
                    if(driver)
                    {
                        /* Found fs driver */
                        printf("vfs: rootfs: %s (%s)\n", bpb.rootfs, bpb.fstype);
                        return mount_add("/", driver, dev);
                    }

                    printf("vfs: no driver for rootfs (%s) filesystem '%s'\n",
                           bpb.rootfs, bpb.fstype);
                }
            }
        }

        printf("vfs: rootfs partition '%s' not found\n", bpb.rootfs);
    }
    else
        puts("vfs: rootfs not set in BPB");
#endif

	return SUCCESS;
}


/* Default versions of the functions in vfs_driver_t.  These all return ENOSYS. */
s32 vfs_default_mount(vfs_t *vfs)
{ return ENOSYS; }
s32 vfs_default_umount(vfs_t *vfs)
{ return ENOSYS; }
s32 vfs_default_get_root_dirent(vfs_t *vfs, vfs_dirent_t *dirent)
{ return ENOSYS; }
s32 vfs_default_open_dir(vfs_t *vfs, u32 node, void **ctx)
{ return ENOSYS; }
s32 vfs_default_read_dir(vfs_t *vfs, void *ctx, vfs_dirent_t *dirent, ks8 * const name)
{ return ENOSYS; }
s32 vfs_default_close_dir(vfs_t *vfs, void *ctx)
{ return ENOSYS; }
s32 vfs_default_stat(vfs_t *vfs, fs_stat_t *st)
{ return ENOSYS; }


vfs_driver_t *vfs_get_driver_by_name(ks8 * const name)
{
    s32 i;

    for(i = 0; i < (sizeof(g_fs_drivers) / sizeof(g_fs_drivers[0])); ++i)
    {
        if(!strcmp(g_fs_drivers[i]->name, name))
            return g_fs_drivers[i];
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

    vfs = mount_find(path, &rel);
    if(vfs == NULL)
        return ENOENT;      /* Should only happen if no root fs is mounted */

    node = vfs->root_node;

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
    str[0] = (dirent->type == FSNODE_TYPE_DIR) ? 'd' : '-';
    str[1] = (dirent->permissions & VFS_PERM_UR) ? 'r' : '-';
    str[2] = (dirent->permissions & VFS_PERM_UW) ? 'w' : '-';
    str[3] = (dirent->permissions & VFS_PERM_UX) ?
                ((dirent->permissions & VFS_PERM_UT) ? 's' : 'x') :
                ((dirent->permissions & VFS_PERM_UT) ? 'S' : '-');
    str[4] = (dirent->permissions & VFS_PERM_GR) ? 'r' : '-';
    str[5] = (dirent->permissions & VFS_PERM_GW) ? 'w' : '-';
    str[6] = (dirent->permissions & VFS_PERM_GW) ?
                ((dirent->permissions & VFS_PERM_GT) ? 's' : 'x') :
                ((dirent->permissions & VFS_PERM_GT) ? 'S' : '-');
    str[7] = (dirent->permissions & VFS_PERM_OR) ? 'r' : '-';
    str[8] = (dirent->permissions & VFS_PERM_OW) ? 'w' : '-';
    str[9] = (dirent->permissions & VFS_PERM_OX) ?
                ((dirent->permissions & VFS_PERM_OT) ? 's' : 'x') :
                ((dirent->permissions & VFS_PERM_OT) ? 'S' : '-');

    return str;
}
