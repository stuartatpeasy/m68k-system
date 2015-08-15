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


device_t *find_boot_device()
{
	u32 i;

	for(i = 0; i < MAX_DEVICES; ++i)
	{
        if(g_devices[i] != NULL)
        {
            device_t * const dev = g_devices[i];
            if(dev->type == DEVICE_TYPE_BLOCK)
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
    }

    ret = mount_init();
    if(ret != SUCCESS)
        return ret;

	/* Find rootfs device */
	ret = bbram_param_block_read(&bpb);
	if(ret == SUCCESS)
    {
        /* Iterate over partition devices, looking for one whose name matches the BPB rootfs */
        for(i = 0; i < MAX_DEVICES; ++i)
        {
            if(g_devices[i] != NULL)
            {
                device_t * const dev = g_devices[i];
                if((dev->type == DEVICE_TYPE_BLOCK) && (dev->class == DEVICE_CLASS_PARTITION) &&
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

	return SUCCESS;
}


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

    path_component = (s8 *) kmalloc(NAME_MAX_LEN + 1);
    if(!path_component)
        return ENOMEM;

    node = vfs->root_node;
    do
    {
        for(i = 0; (*rel != DIR_SEPARATOR) && (*rel != '\0'); ++i)
            path_component[i] = *rel++;

        if(!i)
            continue;               /* Skip over empty path components */

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


s8 *vfs_dirent_perm_str(const vfs_dirent_t * const dirent, s8 *str)
{
    /* TODO: handle sticky bits */
    str[0] = (dirent->type == FSNODE_TYPE_DIR) ? 'd' : '-';
    str[1] = (dirent->permissions & VFS_PERM_UR) ? 'r' : '-';
    str[2] = (dirent->permissions & VFS_PERM_UW) ? 'w' : '-';
    str[3] = (dirent->permissions & VFS_PERM_UX) ? 'x' : '-';
    str[4] = (dirent->permissions & VFS_PERM_GR) ? 'r' : '-';
    str[5] = (dirent->permissions & VFS_PERM_GW) ? 'w' : '-';
    str[6] = (dirent->permissions & VFS_PERM_GX) ? 'x' : '-';
    str[7] = (dirent->permissions & VFS_PERM_OR) ? 'r' : '-';
    str[8] = (dirent->permissions & VFS_PERM_OW) ? 'w' : '-';
    str[9] = (dirent->permissions & VFS_PERM_OX) ? 'x' : '-';

    return str;
}
