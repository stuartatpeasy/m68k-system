#ifndef KERNEL_FS_EXT2_EXT2_H_INC
#define KERNEL_FS_EXT2_EXT2_H_INC
/*
	ext2 file system driver

	Part of the as-yet-unnamed MC68010 operating system.


	(c) Stuart Wallace <stuartw@atom.net>, December 2012.
*/

#include <kernel/include/defs.h>
#include <kernel/include/fs/vfs.h>
#include <kernel/include/types.h>
#include <kernel/include/memory/kmalloc.h>
#include <kernel/util/kutil.h>
#include <klibc/include/errno.h>
#include <klibc/include/stdio.h>
#include <klibc/include/string.h>


#define EXT2_SUPER_MAGIC		(0xef53)
#define EXT2_SUPERBLOCK_OFFSET	(1024)

#define EXT2_BAD_INO						(1)			/* Bad blocks inode						*/
#define EXT2_ROOT_INO						(2)			/* Root directory inode					*/
#define EXT2_ACL_IDX_INO					(3)			/* ACL index inode						*/
#define EXT2_ACL_DATA_INO					(4)			/* ACL data inode						*/
#define EXT2_BOOT_LOADER_INO				(5)			/* Boot loader inode					*/
#define EXT2_UNDEL_DIR_INO					(6)			/* Undelete directory inode				*/

#define EXT2_S_IFSOCK						(0xc000)	/* socket								*/
#define EXT2_S_IFLNK						(0xa000)	/* symbolic link						*/
#define EXT2_S_IFREG						(0x8000)	/* regular file							*/
#define EXT2_S_IFBLK						(0x6000)	/* block device							*/
#define EXT2_S_IFDIR						(0x4000)	/* directory							*/
#define EXT2_S_IFCHR						(0x2000)	/* character device						*/
#define EXT2_S_IFIFO						(0x1000)	/* fifo									*/
#define EXT2_S_ISUID						(0x0800)	/* set process uid						*/
#define EXT2_S_ISGID						(0x0400)	/* set process gid						*/
#define EXT2_S_ISVTX						(0x0200)	/* sticky bit							*/
#define EXT2_S_IRUSR						(0x0100)	/* user read							*/
#define EXT2_S_IWUSR						(0x0080)	/* user write							*/
#define EXT2_S_IXUSR						(0x0040)	/* user execute							*/
#define EXT2_S_IRGRP						(0x0020)	/* group read							*/
#define EXT2_S_IWGRP						(0x0010)	/* group write							*/
#define EXT2_S_IXGRP						(0x0008)	/* group execute						*/
#define EXT2_S_IROTH						(0x0004)	/* others read							*/
#define EXT2_S_IWOTH						(0x0002)	/* others write							*/
#define EXT2_S_IXOTH						(0x0001)	/* others execute						*/

#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER	(0x0001)	/* Sparse superblock backups			*/
#define EXT2_FEATURE_RO_COMPAT_LARGE_FILE	(0x0002)	/* Large file support					*/
#define EXT2_FEATURE_RO_COMPAT_BTREE_DIR	(0x0004)	/* Binary tree sorted directory files	*/

#define EXT2_FT_UNKNOWN						(0)			/* Unknown file type					*/
#define EXT2_FT_REG_FILE					(1)			/* Regular file							*/
#define EXT2_FT_DIR							(2)			/* Directory file						*/
#define EXT2_FT_CHRDEV						(3)			/* Character device						*/
#define EXT2_FT_BLKDEV						(4)			/* Block device							*/
#define EXT2_FT_FIFO						(5)			/* FIFO									*/
#define EXT2_FT_SOCK						(6)			/* Socket file							*/
#define EXT2_FT_SYMLINK						(7)			/* Symbolic link						*/

struct ext2_superblock
{
	u32 s_inodes_count;				/* Total number of inodes in the file system				*/
	u32 s_blocks_count;				/* Total number of blocks in the file system				*/
	u32 s_r_blocks_count;			/* # blocks reserved for the superuser						*/
	u32 s_free_blocks_count;		/* # free blocks (incl. reserved blocks)					*/
	u32 s_free_inodes_count;		/* # free inodes											*/
	u32 s_first_data_block;			/* id of the first data block (containing the superblock)	*/
	u32 s_log_block_size;			/* block size = 1024 << s_log_block_size					*/
	u32 s_log_frag_size;			/* fragment size - not widely supported						*/
	u32 s_blocks_per_group;			/* blocks per group											*/
	u32 s_frags_per_group;			/* fragments per group; determines size of block bitmap		*/
	u32 s_inodes_per_group;			/* inodes per group											*/
	u32 s_mtime;					/* timestamp of last mount									*/
	u32 s_wtime;					/* timestamp of last write									*/
	u16 s_mnt_count;				/* # mounts since last check								*/
	u16 s_max_mnt_count;			/* max mounts before check required							*/
	u16 s_magic;					/* =EXT2_SUPER_MAGIC (0xef53)								*/
	u16 s_state;					/* fs state: EXT2_ERROR_FS / EXT2_VALID_FS					*/
	u16 s_errors;					/* what to do when an error is detected						*/
	u16 s_minor_rev_level;			/* minor revision level										*/
	u32 s_lastcheck;				/* timestamp of last fs check								*/
	u32 s_checkinterval;			/* maximum interval between checks							*/
	u32 s_creator_os;				/* identity of the os that created the fs					*/
	u32 s_rev_level;				/* revision level											*/
	u16 s_def_resuid;				/* default user id for reserved blocks						*/
	u16 s_def_resgid;				/* default group id for reserved blocks						*/

	/* EXT2_DYNAMIC_REV-specific fields */
	u32 s_first_ino;
	u16 s_inode_size;
	u16 s_block_group_nr;
	u32 s_feature_compat;
	u32 s_feature_incompat;
	u32 s_feature_ro_compat;
	s8 s_uuid[16];
	s8 s_volume_name[16];
	s8 s_last_mounted[64];
	u32 s_algo_bitmap;

	/* Performance hints */
	u8 s_prealloc_blocks;
	u8 s_prealloc_dir_blocks;
	u16 _align_1;					/* Padding */

	/* Journalling support */
	s8 s_journal_uuid[16];
	u32 s_journal_inum;
	u32 s_journal_dev;
	u32 s_last_orphan;

	/* Directory indexing support */
	u32 s_hash_seed[4];
	u8 s_def_hash_version;
	u8 _align_2[3];					/* Padding */

	/* Other options */
	u32 s_default_mount_options;
	u32 s_first_meta_bg;
} __attribute__((packed));


/* Block group descriptor */
struct ext2_bgd
{
	u32 bg_block_bitmap;
	u32 bg_inode_bitmap;
	u32 bg_inode_table;
	u16 bg_free_blocks_count;
	u16 bg_free_inodes_count;
	u16 bg_used_dirs_count;
	u16 bg_pad;
	u8 bg_reserved[12];
} __attribute__((packed));

#define EXT2_BGD_SIZE		(32)
#define EXT2_LOG_BGD_SIZE	(5)


struct ext2_inode
{
	u16 i_mode;
	u16 i_uid;
	u32 i_size;
	u32 i_atime;
	u32 i_ctime;
	u32 i_mtime;
	u32 i_dtime;
	u16 i_gid;
	u16 i_links_count;
	u32 i_blocks;
	u32 i_flags;
	u32 i_osd1;
	u32 i_block[15];
	u32 i_generation;
	u32 i_file_acl;
	u32 i_dir_acl;
	u32 i_faddr;
	u8 i_osd2[12];
} __attribute__((packed));

#define EXT2_INODE_SIZE		(128)
#define EXT2_LOG_INODE_SIZE	(7)


struct ext2_dirent
{
	u32 inode;
	u16 rec_len;
	u8 name_len;
	u8 file_type;
	u8 name[];
} __attribute__((packed));


struct ext2_filesystem
{
	struct ext2_superblock *sblk;
	struct ext2_bgd *bgd;
	u32 bgd_table_clean:1;				/* 1 = table unmodified; 0 = changes not flushed to disc */
};

typedef struct ext2_superblock ext2_superblock_t;
typedef struct ext2_bgd ext2_bgd_t;
typedef struct ext2_inode ext2_inode_t;
typedef struct ext2_dirent ext2_dirent_t;
typedef struct ext2_filesystem ext2_filesystem_t;

typedef u32 inum_t;


s32 ext2_init();
s32 ext2_mount(vfs_t *vfs);
s32 ext2_umount(vfs_t *vfs);
s32 ext2_get_root_dirent(vfs_t *vfs, vfs_dirent_t *dirent);
s32 ext2_open_dir(vfs_t *vfs, u32 node, void **ctx);
s32 ext2_read_dir(vfs_t *vfs, void *ctx, vfs_dirent_t *dirent, const s8* const name);
s32 ext2_close_dir(vfs_t *vfs, void *ctx);
s32 ext2_stat(vfs_t *vfs, fs_stat_t *st);

u32 block_group_contains_superblock(const ext2_filesystem_t *fs, ku32 block_group);
u32 ext2_read_block(vfs_t *vfs, ku32 block, void **ppbuf);
u32 ext2_read_inode(vfs_t *vfs, u32 inum, ext2_inode_t *inode);
u32 unaligned_read(u32 start, u32 len, void *data);
u32 unaligned_write(u32 start, u32 len, const void *data);
u32 ext2_inode_get_block(vfs_t *vfs, const ext2_inode_t *inode, u32 num, u32 *block);
u32 ext2_parse_path(vfs_t *vfs, ks8 *path, inum_t *inum);
void ext2();

#endif

