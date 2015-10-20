/*
	File-handling functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, August 2015.
*/

#include "fs/file.h"


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
    ret = vfs_lookup(path, ent);        /* TODO: do stat() instead */
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
