/*
    Mount point functions

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace <stuartw@atom.net>, July 2012.
*/

#include <kernel/include/defs.h>
#include <kernel/include/fs/mount.h>
#include <kernel/include/fs/vfs.h>
#include <kernel/include/lock.h>
#include <kernel/include/memory/slab.h>
#include <kernel/util/kutil.h>
#include <klibc/include/errno.h>


mount_ent_t *g_mount_table = NULL;


s32 mount_init()
{
    /* Nothing to do here (yet) */
    return SUCCESS;
}


/*
    mount_add() - mount the file system specified by <driver> and <dev> at the location specified by
    <host_vfs>:<host_node>.
*/
s32 mount_add(vfs_t * const host_vfs, fs_node_t * const host_node, vfs_driver_t * const driver,
              dev_t * const dev)
{
    s32 ret;
    mount_ent_t *ent, *new_ent;
    vfs_t *inner_vfs;

    /* TODO: auto-detect driver, if driver == NULL */

    if(driver == NULL)
        return EINVAL;      /* No driver auto-detection yet */

    /*
        Validate args: either host_vfs and host_node must be NULL (implying that we are mounting the
        root filesystem) or they must both be non-NULL (implying that we are mounting a child fs).
    */
    if(((host_vfs != NULL) && (host_node == NULL)) || ((host_vfs != NULL) && (host_node == NULL)))
        return EINVAL;

    preempt_disable();                              /* BEGIN locked section */

    /*
        Ensure that there is not already a filesystem mounted at host_vfs:host_node, and that dev is
        not already mounted anywhere.
    */
    for(ent = g_mount_table; ent != NULL; ent = ent->next)
        if(((ent->host_vfs == host_vfs) && (ent->host_node == host_node))
            ||(ent->host_vfs->dev == dev) || (ent->inner_vfs->dev == dev))
        {
            preempt_enable();
            return EBUSY;
        }

    ret = vfs_attach(driver, dev, &inner_vfs);      /* Create a new VFS for the mount */
    if(ret != SUCCESS)
    {
        preempt_enable();
        return ret;
    }

    ret = slab_alloc(sizeof(mount_ent_t), (void **) &new_ent);
    if(ret != SUCCESS)
    {
        vfs_detach(inner_vfs);
        preempt_enable();
        return ret;
    }

    new_ent->host_vfs  = host_vfs;
    new_ent->host_node = host_node;
    new_ent->inner_vfs = inner_vfs;
    new_ent->next      = NULL;

    if(g_mount_table == NULL)
        g_mount_table = new_ent;
    else
        ent->next = new_ent;

    preempt_enable();                               /* END locked section */

    return SUCCESS;

}


/*
    mount_remove() - remove (unmount) a mount.  The mount is specified by the location in
    <host_vfs>:<host_node> and optionally the device <dev>.  If <host_vfs>:<host_node> are both
    NULL, the root filesystem mount is implied.  In this case, <dev> must be NULL or must match the
    device mounted at the filesystem root.
*/
s32 mount_remove(const vfs_t * const host_vfs, const fs_node_t * const host_node,
                 const dev_t * const dev)
{
    mount_ent_t *ent, *ent_prev;

    /*
        If a location is specified by <host_vfs>:<host_node>, both must be non-NULL.  Otherwise,
        both must be NULL.
    */
    if(((host_vfs != NULL) && (host_node == NULL)) || ((host_vfs != NULL) && (host_node == NULL)))
        return EINVAL;

    preempt_disable();
    for(ent = g_mount_table, ent_prev = NULL; ent != NULL; ent_prev = ent, ent = ent->next)
    {
        if((ent->host_vfs == host_vfs) && (ent->host_node == host_node))
        {
            s32 ret;

            if((dev != NULL) && (dev != ent->inner_vfs->dev))
            {
                preempt_enable();
                return ENOENT;      /* Incorrect device specified */
            }

            /* Unmount the filesystem */
            ret = vfs_unmount(ent->inner_vfs);
            if(ret != SUCCESS)
            {
                preempt_enable();
                return ret;
            }

            ret = vfs_detach(ent->inner_vfs);
            if(ret != SUCCESS)
            {
                preempt_enable();
                return ret;
            }

            if(ent_prev != NULL)
                ent_prev->next = ent->next;
            else
                g_mount_table = ent->next;

            slab_free(ent);
            preempt_enable();

            return SUCCESS;
        }
    }

    preempt_enable();

    return ENOENT;      /* Mount not found */
}


/*
    mount_find() - determine whether the specified VFS (<host_vfs>) and node (<host_node>)
    corresponds to a mount point; if so, return the mounted VFS and root node through <*inner_vfs>
    and <*inner_node> if these parameters are non-NULL.  Return ENOENT if the supplied arguments do
    not refer to a mount point.
*/
s32 mount_find(const vfs_t * const host_vfs, const fs_node_t * const host_node, vfs_t **inner_vfs,
               fs_node_t **inner_node)
{
    mount_ent_t *mnt;
    s32 ret;

    preempt_disable();              /* BEGIN locked section */

    for(mnt = g_mount_table; mnt != NULL; mnt = mnt->next)
    {
        if((mnt->host_vfs == host_vfs) && (mnt->host_node == host_node))
        {
            if(inner_node != NULL)
            {
                ret = mnt->inner_vfs->driver->get_root_node(mnt->inner_vfs, inner_node);
                if(ret != SUCCESS)
                {
                    preempt_enable();
                    return ret;
                }
            }

            if(inner_vfs != NULL)
                *inner_vfs = mnt->inner_vfs;

            preempt_enable();

            return SUCCESS;
        }
    }

    preempt_enable();               /* END locked section */

    return ENOENT;
}

