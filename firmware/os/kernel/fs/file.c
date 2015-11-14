/*
	File-handling functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, August 2015.
*/

#include <kernel/fs/file.h>


/*
    file_open() - open a file
*/
s32 file_open(ks8 * const path, u32 flags, file_info_t *fp)
{
    vfs_dirent_t *ent;
    s32 ret;

    ent = (vfs_dirent_t *) kmalloc(sizeof(vfs_dirent_t));
    if(!ent)
        return ENOMEM;

    /* Check that the file exists */
    ret = vfs_lookup(path, ent);
    if(ret == SUCCESS)
    {
        /* File exists.  Was exclusive creation requested? */
        if(flags & O_EXCL)
        {
            kfree(ent);
            return EEXIST;
        }

        /* Check perms */

        fp->dirent = ent;
        fp->flags = flags;
        fp->offset = (flags & O_APPEND) ? ent->size : 0;

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
            kfree(ent);
            return ENOENT;      /* File does not exist */
        }
    }
    else
    {
        kfree(ent);
        return ret;     /* Something went wrong in vfs_lookup() */
    }
}


/*
    file_check_perms() - check that a user can perform the requested operation (read, write,
    execute) on the supplied dirent.
*/
s32 file_check_perms(uid_t uid, const file_perm_t op, const vfs_dirent_t * const ent)
{
    file_perm_t perm;

    if(uid == ROOT_UID)                     /* User is root */
    {
        /*
            Root user can read and write anything, but can only execute files with at least one
            execute permission bit set.
        */
        perm = VFS_PERM_R | VFS_PERM_W;
        if(ent->permissions & (VFS_PERM_UX | VFS_PERM_GX | VFS_PERM_OX))
            perm |= VFS_PERM_X;
    }
    else if(uid == ent->uid)                /* If UID matches, use user perms */
        perm = ent->permissions >> VFS_PERM_SHIFT_U;
    else if(group_member(uid, ent->gid))    /* User is in file's group; use group perms */
        perm = ent->permissions >> VFS_PERM_SHIFT_G;
    else                                    /* Use "other" perms */
        perm = ent->permissions >> VFS_PERM_SHIFT_O;

    perm &= VFS_PERM_MASK;

    return ((perm & op) == op) ? SUCCESS : EPERM;
}
