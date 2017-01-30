/*
	Mount point functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, July 2012.
*/

#include <klibc/include/errno.h>
#include <kernel/include/fs/mount.h>
#include <kernel/include/defs.h>
#include <kernel/include/memory/kmalloc.h>
#include <kernel/util/kutil.h>


mount_ent_t *g_mount_table = NULL;


s32 mount_init()
{
    /* Nothing to do here (yet) */
	return SUCCESS;
}


s32 mount_add(const char * const mount_point, vfs_driver_t *driver, dev_t *dev)
{
	s32 ret;
	mount_ent_t *new_ent;

    /* TODO: look up driver, if none is supplied */
    if((mount_point == NULL) || (driver == NULL) || (dev == NULL))
        return EINVAL;

    new_ent = CHECKED_KMALLOC(sizeof(mount_ent_t));

	new_ent->mount_point = strdup(mount_point);
	if(!new_ent->mount_point)
    {
        kfree(new_ent);
		return ENOMEM;		/* strdup() failed - OOM */
    }

    new_ent->vfs = kmalloc(sizeof(vfs_t));
    if(!new_ent->vfs)
    {
        kfree(new_ent->mount_point);
        kfree(new_ent);
        return ENOMEM;
    }

    new_ent->next = NULL;
	new_ent->mount_point_len = strlen(mount_point);         /*  FIXME - why on earth is this needed? */

    new_ent->vfs->dev = dev;
    new_ent->vfs->driver = driver;

    /* Attempt to mount the filesystem */
    ret = new_ent->vfs->driver->mount(new_ent->vfs);
    if(ret != SUCCESS)
    {
        kfree(new_ent->mount_point);
        kfree(new_ent->vfs);
        kfree(new_ent);

        return ret;
    }

    if(g_mount_table)
    {
        mount_ent_t *e;
        for(e = g_mount_table; e->next != NULL; e = e->next)
            ;

        e->next = new_ent;
    }
    else
        g_mount_table = new_ent;    /* This is the first entry in the mount table */

    return SUCCESS;
}


s32 mount_remove(const char * const mount_point)
{
    mount_ent_t *ent, *ent_prev;

    for(ent = g_mount_table, ent_prev = NULL; ent; ent_prev = ent, ent = ent->next)
    {
        if(!strcmp(mount_point, ent->mount_point))
        {
			/* TODO: signal the device that it is being unmounted; flush changed pages, etc */
            ent_prev->next = ent->next;
            kfree(ent->mount_point);
            kfree(ent->vfs);
            kfree(ent);

            return SUCCESS;
        }
    }

	return ENOENT;		/* no such mount point */
}


/*
    mount_find() - given an absolute (but not necessarily complete) path, find and return the VFS
    under which the path is mounted.  If rel is non-NULL, point it at the first character past the
    mountpoint.
*/
vfs_t *mount_find(const char * const path, const char **rel)
{
	u32 best_match_len = 0;
	vfs_t *fs = NULL;
	mount_ent_t *ent;

	for(ent = g_mount_table; ent; ent = ent->next)
    {
        ku32 len = ent->mount_point_len;

        if(!strncmp(path, ent->mount_point, len) && (len > best_match_len))
        {
            fs = ent->vfs;
            best_match_len = len;
        }
    }

    if(rel)
        *rel = path + best_match_len;

    return fs;  /* returns NULL if no root fs is mounted */
}

