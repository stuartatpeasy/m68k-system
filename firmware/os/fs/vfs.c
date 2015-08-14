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
	for(i = 0; i < (sizeof(g_fs_drivers) / sizeof(g_fs_drivers[0])); ++i)
    {
        printf("vfs: initialising '%s' fs driver: ", g_fs_drivers[i]->name);
        if(g_fs_drivers[i]->init() == SUCCESS)
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


s32 vfs_lookup(ks8 * path, vfs_dirent_t *ent)
{
    vfs_t *vfs;
    const char *rel;
    s8 *path_component;
    void *ctx;
    u32 i, node;
    s32 ret;

    vfs = mount_find(path, &rel);
    if(vfs == NULL)
        return ENOENT;      /* Should only happen if no root fs is mounted */

    path_component = (s8 *) kmalloc(NAME_MAX_LEN) + 1;
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

        if(*rel == DIR_SEPARATOR)
            printf("component: %s (dir)\n", path_component);
        else
            printf("component: %s (file)\n", path_component);

        ret = vfs->driver->read_dir(vfs, ctx, ent, path_component);
        if(ret != SUCCESS)
        {
            vfs->driver->close_dir(vfs, ctx);
            kfree(path_component);
            return ret;
        }

        vfs->driver->close_dir(vfs, ctx);

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


s32 vfs_close_dir(void *ctx)
{
    /* TODO */
    return SUCCESS;
}


