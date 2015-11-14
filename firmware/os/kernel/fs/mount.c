/*
	Mount point functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, July 2012.
*/

#include <errno.h>
#include <include/defs.h>
#include <kernel/fs/mount.h>
#include <kutil/kutil.h>
#include <memory/kmalloc.h>

mount_ent_t *g_mount_table = NULL;
u32 g_max_mounts = 0;
u32 g_mount_end = 0;


/* Allocate how_many new entries in the mount table.  This is done carefully to avoid the
   possibility that a failed realloc() might cause the loss of the entire table. */
s32 expand_mount_table(ku32 how_many)
{
	mount_ent_t *new_table;

	new_table = kmalloc(sizeof(mount_ent_t) * (g_max_mounts + how_many));
	if(!new_table)
		return ENOMEM;

	/* Copy the contents of the old table into the new table */
	memcpy(new_table, g_mount_table, sizeof(mount_ent_t) * g_max_mounts);

	/* Zero the extra space created in the new table */
	bzero(&new_table[g_max_mounts], sizeof(mount_ent_t) * how_many);

	/* Release the old table and point g_mount_table at the new table */
	kfree(g_mount_table);
	g_mount_table = new_table;

	g_max_mounts += how_many;

	return SUCCESS;
}


s32 mount_init()
{
	s32 ret;

	g_max_mounts = 0;

	ret = expand_mount_table(MOUNT_INITIAL_NUM_MOUNT_POINTS);
	if(ret)
		return ret;	/* probably ENOMEM */

	return SUCCESS;
}


s32 mount_add(const char * const mount_point, vfs_driver_t *driver, dev_t *dev)
{
	s32 ret;
	u32 u;
	mount_ent_t *ent;

	if(g_mount_end == g_max_mounts)
	{
		/* No more mount points available.  Try to expand the table */
		ks32 ret = expand_mount_table(MOUNT_ADDITIONAL_MOUNT_POINTS);
		if(ret)
			return ret;		/* probably ENOMEM */
	}

	for(u = 0; u < g_mount_end; ++u)
	{
		if(!strcmp(mount_point, g_mount_table[u].mount_point))
			return EBUSY;	/* Something else is already mounted here */
	}

	ent = &(g_mount_table[g_mount_end]);

	ent->mount_point = strdup(mount_point);
	ent->mount_point_len = strlen(mount_point);
	if(!ent->mount_point)
		return ENOMEM;		/* strdup() failed - OOM */

    ent->vfs = kmalloc(sizeof(vfs_t));
    if(!ent->vfs)
    {
        kfree(ent->mount_point);
        ent->mount_point = NULL;
        return ENOMEM;
    }

    ent->vfs->dev = dev;
    ent->vfs->driver = driver;

    /* Attempt to mount the filesystem */
    ret = ent->vfs->driver->mount(ent->vfs);
    if(ret != SUCCESS)
    {
        kfree(ent->mount_point);
        kfree(ent->vfs);
        ent->mount_point = NULL;
        ent->vfs = NULL;

        return ret;
    }

	++g_mount_end;

	return SUCCESS;
}


s32 mount_remove(const char * const mount_point)
{
	u32 u;
	for(u = 0; u < g_mount_end; ++u)
	{
		if(!strcmp(mount_point, g_mount_table[u].mount_point))
		{
			/* TODO: signal the device that it is being unmounted; flush changed pages, etc */
			memcpy(&g_mount_table[u], &g_mount_table[u + 1],
						(g_mount_end - u - 1) * sizeof(mount_ent_t));

			--g_mount_end;
			return SUCCESS;
		}
	}

	return EINVAL;		/* no such mount point */
}


/*
    mount_find() - given an absolute (but not necessarily complete) path, find and return the VFS
    under which the path is mounted.  If rel is non-NULL, point it at the first character past the
    mountpoint.
*/
vfs_t *mount_find(const char * const path, const char **rel)
{
	u32 u, best_match_len = 0;
	vfs_t *fs = NULL;

	for(u = 0; u < g_mount_end; ++u)
	{
        const mount_ent_t * const ent = &(g_mount_table[u]);
        const u32 len = ent->mount_point_len;

		if(!strncmp(path, ent->mount_point, len) && (len > best_match_len))
		{
			fs = ent->vfs;
			best_match_len = len;
		}
	}

    if(rel != NULL)
        *rel = path + best_match_len;

	return fs;  /* Should never return NULL, unless no root fs is mounted */
}

