#ifndef KERNEL_FS_FILE_H_INC
#define KERNEL_FS_FILE_H_INC
/*
	File-handling functions

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, August 2015.
*/

#include <kernel/fs/vfs.h>
#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/include/user.h>


/* file_open() flags */
#define O_RD            (0x01)      /* Open file for reading                                */
#define O_WR            (0x02)      /* Open file for writing                                */
#define O_CREATE        (0x04)      /* Create file if it does not exist                     */
#define O_APPEND        (0x08)      /* Set offset to EOF                                    */
#define O_EXCL          (0x10)      /* Ensure that the call to file_open() creates the file */

#define O_RDWR (O_RD | O_WR)        /* Open file for reading and writing */


struct file_info
{
    vfs_dirent_t *dirent;
    u32 flags;
    u32 offset;
};

typedef struct file_info file_info_t;


s32 file_open(ks8 * const path, u32 flags, file_info_t *fp);
s32 file_check_perms(uid_t uid, const file_perm_t op, const vfs_dirent_t * const ent);


#endif
