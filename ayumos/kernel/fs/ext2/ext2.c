/*
	ext2 file system driver

	Part of the as-yet-unnamed MC68010 operating system.


	(c) Stuart Wallace <stuartw@atom.net>, December 2012.


	TODO: check sblk.s_state
	TODO: cache block group descriptor tables
	TODO: implement ext2_flush_bgd_table(const u32 write_backups_too)
	TODO: allocate block structs dynamically
	TODO: use inum_t everywhere instead of u32
	TODO: sort out char signedness (*sigh*)
	TODO: allocate block buffers dynamically
	TODO: respect permissions

	TODO: formalise calling/return convention:
			- if a function returns a single simple data item (e.g. an int),
				- and cannot fail, it must return the data item (Type 1a function)
				- and can fail in exactly one way, and the values -1 (in the case of functions
				  returning numeric types) or NULL (in the case of functions returning pointers)
				  could not be returned in the case of success, the function must return its
				  result and indicate an error condition by returning -1 or NULL (as appropriate)
				  (Type 1b function)
			- otherwise, the function must return its result through one or more arguments.  In
			  this case, arguments must be grouped such that all input args occur before the first
			  output arg.  The function must return u32 zero on success, or a value from
			  include/errno.h on failure.  (Type 2 function)

			- In the case of an error, type 2 functions must not modify output arguments.

	N.B. This code will fail (probably badly) if the OS block size (BLOCK_SIZE in include/defs.h)
	     is greater than the file system block size.
*/

#ifdef WITH_FS_EXT2

#include <kernel/include/device/block.h>
#include <kernel/fs/ext2/ext2.h>
#include <klibc/include/strings.h>


s32 ext2_init();
s32 ext2_mount(vfs_t *vfs);
s32 ext2_umount(vfs_t *vfs);
s32 ext2_get_root_node(vfs_t *vfs, fs_node_t *node);
s32 ext2_open_dir(vfs_t *vfs, u32 node, void **ctx);
s32 ext2_read_dir(vfs_t *vfs, void *ctx, ks8* const name, fs_node_t *node);
s32 ext2_close_dir(vfs_t *vfs, void *ctx);
s32 ext2_stat(vfs_t *vfs, fs_stat_t *st);


vfs_driver_t g_ext2_ops =
{
    .name = "ext2",
    .init = ext2_init,
    .mount = ext2_mount,
    .umount = ext2_umount,
    .get_root_node = ext2_get_root_node,
    .open_dir = ext2_open_dir,
    .read_dir = ext2_read_dir,
    .close_dir = ext2_close_dir,
    .stat = ext2_stat
};


/**
 * ext2_init - initialise ext2 file system driver
 */
s32 ext2_init()
{
    /* Nothing to do here */
    return SUCCESS;
}


/**
 * ext2_mount - attempt to mount an ext2 file system
 * @vfs: partially-populated vfs_t data structure
 */
s32 ext2_mount(vfs_t *vfs)
{
	ext2_fs_t *fs;
	u32 ret, buf_size, num_block_groups;
	u8 *buf;
    ku32 sblk_nblocks = (sizeof(ext2_superblock_t) + BLOCK_SIZE - 1) / BLOCK_SIZE;

    /* Allocate a buffer large enough to hold the number of blocks occupied by the superblock */
    buf = kmalloc(sblk_nblocks * BLOCK_SIZE);
    if(!buf)
        return ENOMEM;

    ret = block_read_multi(vfs->dev, 1024 / BLOCK_SIZE, sblk_nblocks, buf);

	if(ret != SUCCESS)
	{
	    kfree(buf);
		return ret;
	}

    fs = (ext2_fs_t *) kmalloc(sizeof(ext2_fs_t));
    if(!fs)
    {
        kfree(buf);
        return ENOMEM;
    }

	/*
		Read the superblock
	*/

	fs->sblk = kmalloc(sizeof(ext2_superblock_t));
	if(!fs->sblk)
    {
        kfree(fs);
        kfree(buf);
		return ENOMEM;
    }

	memcpy(fs->sblk, buf, sizeof(ext2_superblock_t));
	kfree(buf);

	if(LE2N16(fs->sblk->s_magic) != EXT2_SUPER_MAGIC)
	{
		kfree(fs->sblk);
		kfree(fs);

		return EBADSBLK;	/* bad superblock (invalid magic number) */
	}

	/* TODO: more superblock validation */
	/* TODO: check compat/incompat flags */

	/*
		Read the block group descriptor table
	*/

    dump_hex(fs->sblk, 1, 0, sizeof(ext2_superblock_t));

	num_block_groups = LE2N32(fs->sblk->s_inodes_count) / LE2N32(fs->sblk->s_inodes_per_group);

	/* Calculate the size of a buffer to hold the BGD table, rounding up to the nearest block */
	buf_size = ((num_block_groups * EXT2_BGD_SIZE) + BLOCK_SIZE - 1) & ~(BLOCK_SIZE - 1);

	if(!(buf = kmalloc(buf_size)))
	{
		kfree(fs->sblk);
	    kfree(fs);
		return ENOMEM;
	}

    ret = block_read_multi(vfs->dev,
                           (1024 << LE2N32(fs->sblk->s_log_block_size)) >> LOG_BLOCK_SIZE,
                           buf_size >> LOG_BLOCK_SIZE,
                           buf);

	if(ret)
	{
	    kfree(buf);
		kfree(fs->sblk);
	    kfree(fs);
		return ret;
	}

	fs->bgd = (struct ext2_bgd *) buf;
	fs->bgd_table_clean = 1;

    vfs->data = fs;

	return SUCCESS;
}


/**
 * ext2_umount - attempt to unmount an ext2 file system
 * @vfs: vfs_t data structure specifying the file system
 */
s32 ext2_umount(vfs_t *vfs)
{
    /* TODO */
    UNUSED(vfs);

    return SUCCESS;
}


s32 ext2_open_dir(vfs_t *vfs, u32 node, void **ctx)
{
    /* TODO */
    UNUSED(vfs);
    UNUSED(node);
    UNUSED(ctx);

    return SUCCESS;
}


s32 ext2_read_dir(vfs_t *vfs, void *ctx, ks8* const name, fs_node_t *node)
{
    /* TODO */
    UNUSED(vfs);
    UNUSED(ctx);
    UNUSED(node);
    UNUSED(name);

    return SUCCESS;
}


s32 ext2_close_dir(vfs_t *vfs, void *ctx)
{
    UNUSED(vfs);
    UNUSED(ctx);

    /* TODO */
    return SUCCESS;
}


s32 ext2_stat(vfs_t *vfs, fs_stat_t *st)
{
    /* TODO */
    UNUSED(vfs);
    UNUSED(st);

    return SUCCESS;
}


/*
	Return nonzero if the specified block group should contain a superblock backup.
*/
u32 block_group_contains_superblock(const ext2_fs_t *fs, ku32 block_group)
{
	/* Originally superblock backups were stored in every block group.  The "sparse superblock"
	   feature saves space by storing backups in block groups 0, 1, and powers of 3, 5 and 7. */
	if(LE2N32(fs->sblk->s_feature_ro_compat) & EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER)
	{
		/* This file system has the "sparse superblock" feature */

		/* Trivial case: backups are stored in block groups 0 and 1 */
		if(block_group < 2)
		{
			return 1;
		}
		else
		{
			/* The limits[] array contains the highest powers of 3, 5 and 7 that are representable in a
			   32-bit uint.  e.g. limits[0] = 20, meaning that 3^20 fits in 32 bits but 3^21 does not. */
			u32 limits[3] = {20, 13, 11}, i;

			/* Check for powers of 3, 5 and 7. */
			for(i = 0; i < (sizeof(limits) / sizeof(limits[0])); ++i)
			{
				ku32 base = (i << 1) + 1;	/* base = {3, 5, 7} */
				u32 mult;

				for(mult = base; (mult < block_group) && limits[i]--; mult *= base)
					;	/* nothing */

				if(mult == block_group)
				{
					return 1;
				}
			}
		}

		return 0;
	}
	else
	{
		/* This file system does not have the "sparse superblock" feature.  Every block group has
		   a superblock backup. */
		return 1;
	}
}


/*
	Read a block into the buffer at *ppbuf.  If *ppbuf is NULL, allocate a new buffer and store
	a pointer to it in *ppbuf.

	************************ TODO - deprecate this function ******************************
*/
u32 ext2_read_block(vfs_t *vfs, ku32 block, void **ppbuf)
{
	u32 ret;
	u8 *buf;
	u32 len;

    const ext2_fs_t *fs = (const ext2_fs_t *) vfs->data;

	/* allocate a new buffer if needed */
	if(*ppbuf == NULL)
	{
		buf = kmalloc(1024 << fs->sblk->s_log_block_size);
		if(buf == NULL)
		{
			return ENOMEM;
		}
	}
	else
	{
		buf = *ppbuf;
	}

printf("[read block %u]\n", block);
    len = (1024 << fs->sblk->s_log_block_size) >> LOG_BLOCK_SIZE;
	ret = vfs->dev->read(vfs->dev, (block << (10 + fs->sblk->s_log_block_size)) >> LOG_BLOCK_SIZE,
							&len, buf);

    /* TODO - check for short read */
	if(ret)
	{
		/* block read failed */
		kfree(buf);
	}
	else
	{
		*ppbuf = buf;
	}

	return ret;
}


/*
	Read the specified inode
*/
u32 ext2_read_inode(vfs_t *vfs, u32 inum, ext2_inode_t *inode)
{
    const ext2_fs_t *fs = (const ext2_fs_t *) vfs->data;
    u32 one = 1;

	if(inum < fs->sblk->s_inodes_count)
	{
		u32 block, offset, ret;
		u8 *buf;

		if(!(buf = kmalloc(BLOCK_SIZE)))
			return ENOMEM;

		--inum;	/* because the first inode in the first block group is 1, not 0 */

		/* offset = offset of this inode from the start of the group */
		offset = (inum % fs->sblk->s_inodes_per_group) << EXT2_LOG_INODE_SIZE;

		/* block = first block of the block group containing the inode + offset of the block
		   containing the inode */
		block = (fs->bgd[inum / fs->sblk->s_inodes_per_group].bg_inode_table
					<< (10 + fs->sblk->s_log_block_size - LOG_BLOCK_SIZE))
				+ (offset >> LOG_BLOCK_SIZE);

		/* inum_ = offset of this inode from the start of the block */
		offset &= BLOCK_SIZE - 1;

		ret = vfs->dev->read(vfs->dev, block, &one, buf);
		if(ret)
			return ret;

		memcpy(inode, buf + offset, sizeof(ext2_inode_t));
		kfree(buf);

		return SUCCESS;
	}

	return EINVAL; /* inum is out of bounds */
}


/*
	Return the block ID of the num'th block of an inode
	XXX UNTESTED
*/
u32 ext2_inode_get_block(vfs_t *vfs, const ext2_inode_t *inode, u32 num, u32 *block)
{
	u32 ret, block_id = 0;
	u32 *buf = NULL;

    ext2_fs_t *fs = (ext2_fs_t *) vfs->data;

	/* Each indirect block stores some number of block IDs.  The size of a block ID is fixed (4
	   bytes) so the number stored per block is proportional to the size of the block.  In order
	   to "dereference" indirect block IDs, it is necessary to calculate a mask that can be used
	   when bit-shifting the block ID.  The mask is:

	   		block_mask = (block_size / sizeof(block ID)) - 1

		i.e.
	*/
	ku32 block_mask = ((1024 << fs->sblk->s_log_block_size) >> 2) - 1;

	/*
		It is also necessary to calculate how many bits to shift an indirect block ID in order to
		obtain its position in a block table.  This number is:

			block_shift = log2(block_size / sizeof(block ID))

		Fortunately log2(block_size) is already available in the superblock field s_log_block_size.
		sizeof(block ID) is fixed at 4, so:

			block_shift = log2(block_size) - log2(4)
						= log2(block_size) - 2
	*/
	ku32 block_shift = (fs->sblk->s_log_block_size + 10) - 2;

	if(num < 12)				/* direct blocks */
	{
		*block = inode->i_block[num];
		return SUCCESS;
	}
	else if(num >= (16777216 + 65536 + 256))
	{
		return EINVAL;	/* block ID out of range */
	}

	/* indirect blocks */
	num -= 12;

	if(num >= (65536 + 256))	/* triply-indirect blocks */
	{
		block_id = inode->i_block[14];

		num -= (65536 + 256);	/* adjust num to the range [0, 16777215] */

		if(!block_id)
			return EINVAL;	/* past EOF */

		ret = ext2_read_block(vfs, block_id, (void **) &buf);
		if(ret)
		{
			kfree(buf);
			return ret;
		}

		/* dereference to doubly-indirect block num */
		block_id = buf[(num >> (2 * block_shift)) & block_mask];
		num &= block_mask | (block_mask << block_shift);

		if(!block_id)
		{
			kfree(buf);
			return EINVAL;	/* past EOF */
		}
	}

	if(num >= 256)				/* doubly-indirect blocks */
	{
		/* At this point block_id will point to a block containing doubly-indirect block IDs (if
		   the requested block was triply indirect and the code above has dereferenced it), or will
		   be zero (if the requested block was doubly indirect).  In the latter case, obtain the
		   block table pointer from the inode in the usual way. */
		if(!block_id)
			block_id = inode->i_block[13];

		num -= 256;		/* adjust num to the range [0, 65535] */

		if(!block_id)
		{
			kfree(buf);
			return EINVAL;	/* past EOF */
		}

		ret = ext2_read_block(vfs, block_id, (void **) &buf);
		if(ret)
		{
			kfree(buf);
			return ret;
		}

		/* dereference to singly-indirect block num */
		block_id = buf[(num >> block_shift) & block_mask];
		num &= block_mask << block_shift;

		if(!block_id)
		{
			kfree(buf);
			return EINVAL;	/* past EOF */
		}

	}

	/* singly-indirect blocks */

	/* At this point block_id will point to a block containing singly-indirect block IDs (if any
	   dereferencing has been done by the code above) or zero (if the requested block was singly
	   indirect).  In the latter case, obtain the block table pointer from the inode in the usual
	   way. */
	if(!block_id)
		block_id = inode->i_block[12];

	ret = ext2_read_block(vfs, block_id, (void **) &buf);
	if(ret)
	{
		kfree(buf);
		return ret;
	}

	*block = buf[num];
	kfree(buf);

	return SUCCESS;
}


/*
	Find the inode containing the file at the specified (absolute) path and return its number.
*/
u32 ext2_parse_path(vfs_t *vfs, ks8 *path, inum_t *inum)
{
	ks8 *end;
	u8 *buf = NULL;
	ext2_inode_t inode;
	inum_t in = EXT2_ROOT_INO;
	u8 name[NAME_MAX_LEN + 1];

	ext2_fs_t *fs = (ext2_fs_t *) vfs->data;

	/* Fail if the first character of the path is anything other than a directory separator (/).
	   e.g. the path is non-absolute, or is empty */
	if(*path != DIR_SEPARATOR)
	{
		return ENOENT;
	}

	for(; *path; path = end)
	{
		u32 data_block, block_index, ret;

		/* find the next dir sep or \0 */
		for(end = ++path; *end && (*end != DIR_SEPARATOR); ++end)
			;

		if(path != end)
		{
			const s32 len = end - path;

			if(len > NAME_MAX_LEN)
			{
				kfree(buf);
				return ENAMETOOLONG;
			}

			/* FIXME remove */
			memcpy(name, path, len);
			name[end - path] = '\0';

			ret = ext2_read_inode(vfs, in, &inode);
			if(ret)
			{
				kfree(buf);
				return ret;
			}

			u8 found = 0;
			for(block_index = 0; !found;)
			{
				const ext2_node_t *d_ent;

				ret = ext2_inode_get_block(vfs, &inode, block_index++, &data_block);
				if(ret)
				{
					kfree(buf);
					return ret;
				}

				if(!data_block)
				{
					break;
				}

				ret = ext2_read_block(vfs, data_block, (void **) &buf);
				if(ret)
				{
					kfree(buf);
					return ret;
				}

				/* search the directory for the path component */
				for(d_ent = (ext2_node_t *) buf;
						(u8 *) d_ent < (buf + (1024 << fs->sblk->s_log_block_size));
						d_ent = (ext2_node_t *) ((u8 *) d_ent + d_ent->rec_len))
				{
					if(!memcmp(d_ent->name, name, len) && !d_ent->name[len])
					{
						if(*end && (d_ent->file_type != EXT2_FT_DIR))
						{
							kfree(buf);
							return ENOTDIR;
						}

						in = d_ent->inode;
						found = 1;	/* ugh */
						break;
					}
				}
			}

			if(!found)
			{
				kfree(buf);
				return ENOENT;
			}
		}
	}

	kfree(buf);
	*inum = in;

	return SUCCESS;
}


s32 ext2_get_root_node(vfs_t *vfs, fs_node_t *node)
{
    const ext2_fs_t * const fs = (const ext2_fs_t *) vfs->data;

    /* Zero out the node struct - that way we only have to set nonzero fields */
    bzero(node, sizeof(fs_node_t));

    node->name[0] = DIR_SEPARATOR;
    node->type = FSNODE_TYPE_DIR;
    node->permissions = FS_PERM_UGORWX;       /* FIXME - read this */

    /* FIXME - 32 bit overflow */
    node->size = LE2N32(fs->sblk->s_blocks_count) << (LE2N32(fs->sblk->s_log_block_size) + 9);
    node->first_block = EXT2_ROOT_INO;

    return SUCCESS;
}

#if 0
/* TODO: express read offsets (args to block_read()) in terms of blocks/sectors, not as magic numbers */
void ext2()
{
	u32 ret, block_num;
	inum_t inum;
	ext2_fs_t *fs;
	u8 *buf = NULL;

	if(ext2_init_filesystem(&fs))
	{
		printf("failed to init fs\n");
		return;
	}

	ret = ext2_parse_path(fs, (ks8 *) "/foo/bar/baz/meh.txt", &inum);
	if(ret)
		printf("ext2_parse_path() failed: %s\n", strerror(ret));
	else
	{
		ext2_inode_t inode;

		printf("ext2_parse_path() returned %u\n", (u32) inum);
		puts("Contents of first data block:");

		ret = ext2_read_inode(fs, inum, &inode);
		if(ret)
		{
			printf("ext2_read_inode() failed: %s\n", strerror(ret));
		}
		else
		{
//			ext2_dump_inode(inode);

			ret = ext2_inode_get_block(fs, &inode, 0, &block_num);
			if(ret)
			{
				printf("ext2_inode_get_block() failed: %s\n", strerror(ret));
			}
			else
			{
				ext2_read_block(fs, block_num, &buf);
				puts((const char *) buf);
				puts((const char *) buf);
			}
		}
	}
}

#endif /* WITH_FS_EXT2 */
#endif
