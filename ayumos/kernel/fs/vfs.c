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
s32 vfs_default_umount(vfs_t *vfs);
s32 vfs_default_get_root_node(vfs_t *vfs, fs_node_t *node);
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
            if(NULL == pdrv->umount)        pdrv->umount        = vfs_default_umount;
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


s32 vfs_default_get_root_node(vfs_t *vfs, fs_node_t *node)
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
    vfs_open_dir() - "open" the directory at <node>, i.e. ensure that <node> represents a directory,
    and prepare to iterate over directory entries.
*/
s32 vfs_open_dir(fs_node_t * const node, vfs_dir_ctx_t **ctx)
{
    vfs_t *vfs;
    vfs_dir_ctx_t *context;
    s32 ret;

    if(node->type != FSNODE_TYPE_DIR)
        return ENOTDIR;

    ret = slab_alloc(sizeof(vfs_dir_ctx_t), (void **) &context);
    if(ret != SUCCESS)
        return ret;

    vfs = node->vfs;
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
s32 vfs_get_root_node(vfs_t *vfs, fs_node_t *node)
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
    vfs_lookup() - look up a path and populate a fs_node_t with the contents.
*/
s32 vfs_lookup(ks8 * path, fs_node_t *node)
{
    vfs_t *vfs;
    const char *rel;
    s8 * path_component;
    void *ctx;
    u32 i, block;
    s32 ret;

    /*
        validate the path: length must be [1, PATH_MAX_LEN]

    */

    if(strlen(path) > NAME_MAX_LEN)
        return ENAMETOOLONG;

    vfs = mount_find(path, &rel);
    if(vfs == NULL)
        return ENOENT;      /* Should only happen if no root fs is mounted */

    block = vfs->root_block;

    for(; *rel == DIR_SEPARATOR; ++rel)
        ;                           /* Skip over empty path components */

    if(rel[0] == '\0')
        return vfs->driver->get_root_node(vfs, node);  /* Special case: root directory */

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

        ret = vfs->driver->open_dir(vfs, block, &ctx);
        if(ret != SUCCESS)
        {
            kfree(path_component);
            return ret;
        }

        ret = vfs->driver->read_dir(vfs, ctx, path_component, node);

        vfs->driver->close_dir(vfs, ctx);
        if(ret != SUCCESS)
        {
            kfree(path_component);
            return ret;
        }

        block = node->first_block;

        /* Check that the located component is a directory or a file, as appropriate */
        if((*rel == DIR_SEPARATOR) && (node->type == FSNODE_TYPE_FILE))
        {
            kfree(path_component);
            return ENOTDIR;         /* Looking for a directory but found a file */
        }

        if((*rel == '\0') && (node->type == FSNODE_TYPE_DIR))
        {
            kfree(path_component);
            return EISDIR;          /* Looking for a file but found a directory */
        }
    } while(*rel++ != '\0');

    kfree(path_component);

    return SUCCESS;
}


/*
    vfs_get_child_node() - populate <*node> with data relating to <child>, a sub-node of <parent>.
    If <parent> is NULL, the fs root directory is implied.  If both <parent> and <child> are NULL,
    <*node> will be populated with data relating to the root directory itself.  If <parent> is non-
    NULL, <child> must also be non-NULL.
*/
s32 vfs_get_child_node(const char *child, fs_node_t *parent, fs_node_t **node)
{
    s32 ret;
    vfs_dir_ctx_t *ctx;
    fs_node_t *parent_;

    if(parent == NULL)
    {
        /* Look for <child> inside the root directory. */
        vfs_t *vfs = mount_find("/", NULL);
        if(vfs == NULL)
            return ENOENT;

        ret = slab_alloc(sizeof(fs_node_t), (void **) &parent_);
        if(ret != SUCCESS)
            return ret;

        ret = vfs_get_root_node(vfs, parent_);
        if(ret != SUCCESS)
        {
            slab_free(parent_);
            return ret;
        }

        if(child == NULL)
        {
            /* Retrieve information about the root directory itself */
            memcpy(*node, parent_, sizeof(fs_node_t));
            slab_free(parent_);

            return SUCCESS;
        }
        else
            printf("root component: %s\n", child);
    }
    else
        parent_ = parent;

    /* It's not valid to supply a NULL child name unless <parent> is also NULL */
putchar('6');
    if(child == NULL)
        return EINVAL;

putchar('7');
    printf("component: %s\n", child);

    ret = vfs_open_dir(parent_, &ctx);
    if(ret != SUCCESS)
        return ret;

    ret = vfs_read_dir(ctx, child, *node);

    vfs_close_dir(ctx);

    return ret;
}

