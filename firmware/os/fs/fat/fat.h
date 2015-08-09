#ifndef FS_FAT_FAT_H_INC
#define FS_FAT_FAT_H_INC
/*
    FAT16 file system support

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, July 2015
*/

#include "include/byteorder.h"
#include "include/defs.h"
#include "include/types.h"
#include "fs/vfs.h"


struct fat_bpb_block
{
    u8  jmp[3];
    s8  oem_id[8];
    u16 bytes_per_sector;
    u8  sectors_per_cluster;
    u16 reserved_sectors;
    u8  fats;
    u16 dir_entries;
    u16 sectors_in_volume;
    u8  media_descriptor_type;
    u16 sectors_per_fat;
    u16 sectors_per_track;
    u16 heads;
    u32 hidden_sectors;
    u32 large_sectors;
    u8  drive_number;
    u8  winnt_flags;
    u8  signature;
    u32 volume_serial_number;
    s8  volume_label[11];
    s8  system_identifier[8];
    u8  boot_code[448];
    u16 partition_signature;
} __attribute__((packed));  /* 512 bytes */


struct fat_dirent
{
    s8  file_name[11];
    u8  attribs;
    u8  winnt_reserved;
    u8  ctime_tenths;
    u16 ctime;
    u16 cdate;
    u16 adate;
    u16 first_cluster_high;
    u16 mtime;
    u16 mdate;
    u16 first_cluster_low;
    u32 size;
} __attribute__((packed));  /* 32 bytes */


#define FAT_FILEATTRIB_READ_ONLY    (0x01)
#define FAT_FILEATTRIB_HIDDEN       (0x02)
#define FAT_FILEATTRIB_SYSTEM       (0x04)
#define FAT_FILEATTRIB_VOLUME_ID    (0x08)
#define FAT_FILEATTRIB_DIRECTORY    (0x10)
#define FAT_FILEATTRIB_ARCHIVE      (0x20)

/* Long filename indicator.  Not supported by this code, but included here anyway... */
#define FAT_FILEATTRIB_LFN          (FAT_FILEATTRIB_READ_ONLY | \
                                     FAT_FILEATTRIB_HIDDEN | \
                                     FAT_FILEATTRIB_SYSTEM | \
                                     FAT_FILEATTRIB_VOLUME_ID)


vfs_driver_t g_fat_ops;

s32 fat_init();
s32 fat_mount(vfs_t *vfs);
s32 fat_umount(vfs_t *vfs);
s32 fat_fsnode_get(vfs_t *vfs, u32 node, fsnode_t * const fsn);
s32 fat_fsnode_put(vfs_t *vfs, u32 node, const fsnode_t * const fsn);
u32 fat_locate(vfs_t *vfs, u32 node, const char * const path);
s32 fat_stat(vfs_t *vfs, fs_stat_t *st);

#endif
