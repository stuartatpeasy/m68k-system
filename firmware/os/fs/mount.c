/*
	Mount point functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, July 2012.
*/

#include <errno.h>
#include "fs/mount.h"
#include "include/defs.h"
#include "kutil/kutil.h"
#include "memory/kmalloc.h"

struct mount_ent *g_mount_table = NULL;
u32 g_max_mounts = 0;
u32 g_mount_end = 0;


/* Allocate how_many new entries in the mount table.  This is done carefully to avoid the
   possibility that a failed realloc() might cause the loss of the entire table. */
s32 expand_mount_table(ku32 how_many)
{
	struct mount_ent *new_table;

	new_table = kmalloc(sizeof(struct mount_ent) * (g_max_mounts + how_many));
	if(!new_table)
		return ENOMEM;

	/* Copy the contents of the old table into the new table */
	memcpy(new_table, g_mount_table, sizeof(struct mount_ent) * g_max_mounts);

	/* Zero the extra space created in the new table */
	bzero(&new_table[g_max_mounts], sizeof(struct mount_ent) * how_many);

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


s32 mount_add(const char * const mount_point, vfs_driver_t *driver, device_t *dev)
{
	s32 i;

	if(g_mount_end == g_max_mounts)
	{
		/* No more mount points available.  Try to expand the table */
		ks32 ret = expand_mount_table(MOUNT_ADDITIONAL_MOUNT_POINTS);
		if(ret)
			return ret;		/* probably ENOMEM */
	}

	for(i = 0; i < g_mount_end; ++i)
	{
		if(!strcmp(mount_point, g_mount_table[i].mount_point))
			return EBUSY;	/* Something else is already mounted here */
	}

	g_mount_table[g_mount_end].mount_point = strdup(mount_point);
	if(!g_mount_table[g_mount_end].mount_point)
		return ENOMEM;		/* strdup() failed - OOM */

	g_mount_table[g_mount_end].device = dev;
	g_mount_table[g_mount_end].driver = driver;

	++g_mount_end;

	return SUCCESS;
}


s32 mount_remove(const char * const mount_point)
{
	s32 i;
	for(i = 0; i < g_mount_end; ++i)
	{
		if(!strcmp(mount_point, g_mount_table[i].mount_point))
		{
			/* TODO: signal the device that it is being unmounted; flush changed pages, etc */
			memcpy(&g_mount_table[i], &g_mount_table[i + 1],
						(g_mount_end - i - 1) * sizeof(struct mount_ent));

			--g_mount_end;
			return SUCCESS;
		}
	}

	return EINVAL;		/* no such mount point */
}


s32 mount_find(const char * const path)
{
	s32 i, best_match = -1, best_match_len = 0;

	for(i = 0; i < g_mount_end; ++i)
	{
		const s32 n = strlen(g_mount_table[i].mount_point);
		if(!strncmp(path, g_mount_table[i].mount_point, n) && (n > best_match_len))
		{
			best_match = i;
			best_match_len = n;
		}
	}

	return best_match;		// -1 if no matching mount point found
}

