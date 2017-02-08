/*
	File-handling functions

	Part of ayumos


	(c) Stuart Wallace <stuartw@atom.net>, August 2015.
*/

#include <kernel/include/fs/file.h>


/*
    file_open() - open a file
*/
s32 file_open(ks8 * const path, u32 flags, file_info_t *fp)
{
    fs_node_t *node;
    s32 ret;

    node = (fs_node_t *) kmalloc(sizeof(fs_node_t));
    if(!node)
        return ENOMEM;

    /* Check that the file exists */
    ret = vfs_lookup(path, node);
    if(ret == SUCCESS)
    {
        /* File exists.  Was exclusive creation requested? */
        if(flags & O_EXCL)
        {
            kfree(node);
            return EEXIST;
        }

        /* Check perms */

        fp->node = node;
        fp->flags = flags;
        fp->offset = (flags & O_APPEND) ? node->size : 0;

        return SUCCESS;
    }
    else if(ret == ENOENT)
    {
        /* File does not exist.  Was creation requested? */
        if(flags & O_CREATE)
        {
            /* Check perms; create file; set offset = 0 */
            /* TODO */
            return ENOSYS;
        }
        else
        {
            kfree(node);
            return ENOENT;      /* File does not exist */
        }
    }
    else
    {
        kfree(node);
        return ret;     /* Something went wrong in vfs_lookup() */
    }
}


/*
    file_check_perms() - check that a user can perform the requested operation (read, write,
    execute) on the supplied node.
*/
s32 file_check_perms(uid_t uid, const file_perm_t op, const fs_node_t * const node)
{
    file_perm_t perm;

    if(uid == ROOT_UID)                     /* User is root */
    {
        /*
            Root user can read and write anything, but can only execute files with at least one
            execute permission bit set.
        */
        perm = FS_PERM_R | FS_PERM_W;
        if(node->permissions & (FS_PERM_UX | FS_PERM_GX | FS_PERM_OX))
            perm |= FS_PERM_X;
    }
    else if(uid == node->uid)                /* If UID matches, use user perms */
        perm = node->permissions >> FS_PERM_SHIFT_U;
    else if(group_member(uid, node->gid))    /* User is in file's group; use group perms */
        perm = node->permissions >> FS_PERM_SHIFT_G;
    else                                    /* Use "other" perms */
        perm = node->permissions >> FS_PERM_SHIFT_O;

    perm &= FS_PERM_MASK;

    return ((perm & op) == op) ? SUCCESS : EPERM;
}
