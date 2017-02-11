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
#include <kernel/include/memory/slab.h>


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


/* Default versions of the functions in vfs_driver_t.  These all return ENOSYS. */
s32 vfs_default_mount(vfs_t *vfs);
s32 vfs_default_unmount(vfs_t *vfs);
s32 vfs_default_get_root_node(vfs_t *vfs, fs_node_t **node);
s32 vfs_default_open_dir(vfs_t *vfs, u32 node, void **ctx);
s32 vfs_default_read_dir(vfs_t *vfs, void *ctx, ks8 * const name, fs_node_t *node);
s32 vfs_default_close_dir(vfs_t *vfs, void *ctx);
s32 vfs_default_stat(vfs_t *vfs, fs_stat_t *st);


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
            if(NULL == pdrv->mount)         pdrv->mount         = vfs_default_mount;
            if(NULL == pdrv->unmount)       pdrv->unmount       = vfs_default_unmount;
            if(NULL == pdrv->get_root_node) pdrv->get_root_node = vfs_default_get_root_node;
            if(NULL == pdrv->open_dir)      pdrv->open_dir      = vfs_default_open_dir;
            if(NULL == pdrv->read_dir)      pdrv->read_dir      = vfs_default_read_dir;
            if(NULL == pdrv->close_dir)     pdrv->close_dir     = vfs_default_close_dir;
            if(NULL == pdrv->stat)          pdrv->stat          = vfs_default_stat;

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

    /* Mount the root filesystem */
    return mount_add(NULL, NULL, driver, dev);
}


/*
    vfs_attach() - attach a new VFS object to the filesystem represented by <driver> and <dev>, then
    return it through <*vfs>.
*/
s32 vfs_attach(vfs_driver_t * const driver, dev_t * const dev, vfs_t **vfs)
{
    vfs_t *new_vfs;
    s32 ret;

    ret = slab_alloc(sizeof(vfs_t), (void **) &new_vfs);
    if(ret != SUCCESS)
        return ret;

    new_vfs->driver = driver;
    new_vfs->dev = dev;
    new_vfs->data = NULL;

    ret = driver->mount(new_vfs);
    if(ret != SUCCESS)
    {
        slab_free(new_vfs);
        return ret;
    }

    *vfs = new_vfs;

    return SUCCESS;
}


/*
    vfs_detach() - detach <vfs> from its device by unmounting it and then freeing its memory.
*/
s32 vfs_detach(vfs_t *vfs)
{
    s32 ret;

    ret = vfs->driver->unmount(vfs);
    if(ret != SUCCESS)
        return ret;

    slab_free(vfs);

    return SUCCESS;
}


/* === Default handlers for the functions in vfs_driver_t.  These all return ENOSYS. === */
s32 vfs_default_mount(vfs_t *vfs)
{
    UNUSED(vfs);

    return ENOSYS;
}


s32 vfs_default_unmount(vfs_t *vfs)
{
    UNUSED(vfs);

    return ENOSYS;
}


s32 vfs_default_get_root_node(vfs_t *vfs, fs_node_t **node)
{
    UNUSED(vfs);
    UNUSED(node);

    return ENOSYS;
}


s32 vfs_default_open_dir(vfs_t *vfs, u32 node, void **ctx)
{
    UNUSED(vfs);
    UNUSED(node);
    UNUSED(ctx);

    return ENOSYS;
}


s32 vfs_default_read_dir(vfs_t *vfs, void *ctx, ks8 * const name, fs_node_t *node)
{
    UNUSED(vfs);
    UNUSED(ctx);
    UNUSED(node);
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
    vfs_unmount() - unmount the device associated with the VFS at <vfs>.
*/
s32 vfs_unmount(vfs_t *vfs)
{
    return vfs->driver->unmount(vfs);
}


/*
    vfs_open_dir() - "open" the directory at <node>, i.e. ensure that <node> represents a directory,
    and prepare to iterate over directory entries.
*/
s32 vfs_open_dir(vfs_t *vfs, fs_node_t * const node, vfs_dir_ctx_t **ctx)
{
    vfs_dir_ctx_t *context;
    s32 ret;

    if(node->type != FSNODE_TYPE_DIR)
        return ENOTDIR;

    ret = slab_alloc(sizeof(vfs_dir_ctx_t), (void **) &context);
    if(ret != SUCCESS)
        return ret;

    context->vfs = vfs;

    ret = vfs->driver->open_dir(vfs, node->first_block, &(context->ctx));
    if(ret != SUCCESS)
    {
        slab_free(context);
        return ret;
    }

    *ctx = context;

    return SUCCESS;
}


/*
    vfs_get_root_node() - get the "root node" (i.e. the root directory) of the supplied VFS; return
    it through <*node>.
*/
s32 vfs_get_root_node(vfs_t *vfs, fs_node_t **node)
{
    return vfs->driver->get_root_node(vfs, node);
}


/*
    vfs_read_dir() - read the next item from a directory "opened" by vfs_open_dir().
*/
s32 vfs_read_dir(vfs_dir_ctx_t *ctx, ks8 * const name, fs_node_t *node)
{
    return ctx->vfs->driver->read_dir(ctx->vfs, ctx->ctx, name, node);
}


/*
    vfs_close_dir() - clean up after iterating over a directory using vfs_open_dir()/vfs_read_dir().
*/
s32 vfs_close_dir(vfs_dir_ctx_t *ctx)
{
    s32 ret;

    ret = ctx->vfs->driver->close_dir(ctx->vfs, ctx->ctx);
    slab_free(ctx);

    return ret;
}


/*
    vfs_get_child_node() - populate <*node> with data relating to <child>, a string containing the
    name of a sub-node of <parent>, on VFS <*vfs>.  If <parent> is NULL, the VFS root directory is
    implied; if both <*vfs> and <parent> are NULL, the file system root directory is implied.  If
    <*vfs>, <parent> and <child> are NULL, <*node> and <*vfs> will be populated with data relating
    to the root directory itself.  If <parent> is non-NULL, <child> must also be non-NULL.

    Behaviour of this function, according to its inputs:

    +==========================+============+======================================+
    |          Inputs          |   In/Out   |                Outputs               |
    +--------------------------+------------+--------------------------------------+
    |  vfs     parent   child  |    vfs     |  node                         error  |
    +==========================+============+======================================+
    | NULL     NULL     NULL   |  <rootfs>  |  <rootfs>/                    -      |
    | NULL     NULL     valid  |  -         |  -                            EINVAL |
    | NULL     valid    NULL   |  -         |  -                            EINVAL |
    | NULL     valid    valid  |  -         |  -                            EINVAL |
    | valid    NULL     NULL   |  <vfs>     |  <vfs>/                       -      |
    | valid    NULL     valid  |  <vfs>     |  <vfs>/<child>                -      |
    | valid    valid    NULL   |  <vfs>     |  -                            EINVAL |
    | valid    valid    valid  |  <vfs>     |  <vfs>.../<parent>/<child>    -      |
    +==========================+============+======================================+

    [*] if <child> is on a different VFS than <parent>, <child>'s VFS will be returned through
        <*vfs>.

    TODO: after finding a node, determine whether it is a mount point.
*/
s32 vfs_get_child_node(fs_node_t *parent, const char * const child, vfs_t **vfs, fs_node_t **node)
{
    s32 ret;
    vfs_dir_ctx_t *ctx;
    fs_node_t *parent_;
    u8 parent_allocated;

    if(*vfs == NULL)
    {
        /* The only valid operation with <*vfs> == NULL is to retrieve the root fs node. */
        if((parent == NULL) && (child == NULL))
        {
            vfs_t *root_fs;
            fs_node_t *root_node;

            ret = mount_find(NULL, NULL, &root_fs, &root_node);
            if(ret != SUCCESS)
                return ret;         /* No root fs - an unusual situation */

            ret = vfs_get_root_node(root_fs, &root_node);
            if(ret == SUCCESS)
            {
                *vfs = root_fs;
                *node = root_node;
            }

            return ret;
        }
        else
            return EINVAL;
    }

    if(parent == NULL)
    {
        /* Null <parent> implies the root dir of <*vfs> */
        ret = vfs_get_root_node(*vfs, &parent_);
        if(ret != SUCCESS)
            return ret;

        parent_allocated = 1;
    }
    else
    {
        /* Non-null <parent> implies a specific dir within <*vfs>.  <child> must be non-null. */
        if(child == NULL)
            return EINVAL;

        parent_ = parent;
        parent_allocated = 0;
    }

    ret = vfs_open_dir(*vfs, parent_, &ctx);
    if(ret != SUCCESS)
    {
        if(parent_allocated)
            fs_node_free(parent_);

        return ret;
    }

    ret = vfs_read_dir(ctx, child, *node);
    vfs_close_dir(ctx);

    if(parent_allocated)
        fs_node_free(parent_);

    return ret;
}
