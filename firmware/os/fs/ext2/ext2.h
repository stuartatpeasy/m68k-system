#ifndef _EXT2_H_
#define _EXT2_H_

#include "ext2-defs.h"

u32 block_group_contains_superblock(const ext2_filesystem_t *fs, ku32 block_group);
u32 ext2_read_block(const ext2_filesystem_t *fs, ku32 block, u8 **ppbuf);
u32 ext2_read_inode(const ext2_filesystem_t *fs, u32 inum, ext2_inode_t *inode);
u32 unaligned_read(u32 start, u32 len, void *data);
u32 unaligned_write(u32 start, u32 len, const void *data);
u32 ext2_init_filesystem(ext2_filesystem_t **ppfs);
u32 ext2_inode_get_block(ext2_filesystem_t *fs, const ext2_inode_t *inode, u32 num, u32 *block);
u32 ext2_parse_path(ext2_filesystem_t *fs, ks8 *path, inum_t *inum);
void ext2();

#endif

