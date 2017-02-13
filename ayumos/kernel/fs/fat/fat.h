#ifndef KERNEL_FS_FAT_FAT_H_INC
#define KERNEL_FS_FAT_FAT_H_INC
/*
    FAT16 file system support

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, July 2015
*/

#ifdef WITH_FS_FAT

#include <kernel/include/byteorder.h>
#include <kernel/include/defs.h>
#include <kernel/include/error.h>
#include <kernel/include/fs/vfs.h>
#include <kernel/include/types.h>
#include <kernel/util/kutil.h>
#include <klibc/include/strings.h>


struct fat_bpb_block
{
    u8  jmp[3];
    s8  oem_id[8];
    u16 bytes_per_sector;
    u8  sectors_per_cluster;
    u16 reserved_sectors;
    u8  fats;
    u16 root_entry_count;
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


/*
    FAT directory entry
*/
#define FAT_FILENAME_LEN        (8)     /* Note: long filenames are dealt with separately */
#define FAT_FILEEXT_LEN         (3)     /* Note: long filenames are dealt with separately */

struct fat_node
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

typedef struct fat_node fat_node_t;


/*
    FAT long filename directory entry
*/
#define FAT_LFN_PART1_LEN       (5)
#define FAT_LFN_PART2_LEN       (6)
#define FAT_LFN_PART3_LEN       (2)
#define FAT_LFN_PART_TOTAL_LEN  (FAT_LFN_PART1_LEN + FAT_LFN_PART2_LEN + FAT_LFN_PART3_LEN)

struct fat_lfn_node
{
    u8 order;
    u16 name_part1[FAT_LFN_PART1_LEN];      /* UTF-16 */
    u8 attribs;
    u8 type;
    u8 checksum;
    u16 name_part2[FAT_LFN_PART2_LEN];      /* UTF-16 */
    u16 reserved;
    u16 name_part3[FAT_LFN_PART3_LEN];      /* UTF-16 */
} __attribute__((packed));

typedef struct fat_lfn_node fat_lfn_node_t;


/*
    This struct contains various useful numbers, computed at mount time.  A struct fat_fs will be
    allocated and filled in fat_mount(), and stored in vfs->data.  It is then deallocated in
    fat_unmount().
*/
struct fat_fs
{
    u32 total_sectors;
    u32 total_data_sectors;
    u32 first_data_sector;
    u32 first_fat_sector;
    u32 root_dir_first_sector;
    u16 total_clusters;
    u16 root_dir_clusters;
    u16 sectors_per_fat;
    u16 sectors_per_cluster;
    u16 bytes_per_cluster;
};

typedef struct fat_fs fat_fs_t;


struct fat_dir_ctx
{
    fat_node_t *buffer;
    fat_node_t *buffer_end;
    u32 block;
    fat_node_t *de;
    s8 is_root_dir;
};

typedef struct fat_dir_ctx fat_dir_ctx_t;


typedef u16 fat16_cluster_id;

#define FAT_ROOT_BLOCK              (0)             /* Only valid in FAT12/FAT16 */

/* Constants used during superblock validation */
#define FAT_PARTITION_SIG           (0xaa55)
#define FAT_JMP_BYTE0               (0xeb)
#define FAT_JMP_BYTE2               (0x90)

/* File attribute constants */
#define FAT_FILEATTRIB_READ_ONLY    (0x01)
#define FAT_FILEATTRIB_HIDDEN       (0x02)
#define FAT_FILEATTRIB_SYSTEM       (0x04)
#define FAT_FILEATTRIB_VOLUME_ID    (0x08)
#define FAT_FILEATTRIB_DIRECTORY    (0x10)
#define FAT_FILEATTRIB_ARCHIVE      (0x20)

/* Long filename indicator */
#define FAT_FILEATTRIB_LFN          (FAT_FILEATTRIB_READ_ONLY | \
                                     FAT_FILEATTRIB_HIDDEN | \
                                     FAT_FILEATTRIB_SYSTEM | \
                                     FAT_FILEATTRIB_VOLUME_ID)

#define FAT_LFN_MAX_LEN             (255)
#define FAT_LFN_ORDER_MASK          (0x1f)  /* AND this with LFN order byte to get LFN order + 1 */

#define FAT_LFN_PART_NUM(order_byte) \
    (((order_byte) & FAT_LFN_ORDER_MASK) - 1)

#define FAT_CHAIN_TERMINATOR        (0xfff7)
#define FAT_CHAIN_END(x)            ((x) >= FAT_CHAIN_TERMINATOR)

#define FAT_DIRENT_END              (0x00)  /* End-of-directory-entries marker  */
#define FAT_DIRENT_UNUSED           (0xe5)  /* Deleted directory entry marker   */

/* FAT-format date/time extraction/conversion macros */
#define FAT_YEAR_FROM_DATE(d)       ((((d) & 0xFE00) >> 9) + 1980)
#define FAT_MONTH_FROM_DATE(d)      (((d) & 0x01E0) >> 5)
#define FAT_DAY_FROM_DATE(d)        ((d) & 0x001F)

#define FAT_HOUR_FROM_TIME(t)       (((t) & 0xF800) >> 11)
#define FAT_MINUTE_FROM_TIME(t)     (((t) & 0x07E0) >> 5)
#define FAT_SECOND_FROM_TIME(t)     (((t) & 0x001F) << 1)

#define FAT_DATE_TO_RTC_DATE(fdate, rdate)                  \
{                                                           \
    (rdate).year = FAT_YEAR_FROM_DATE(fdate);               \
    (rdate).month = FAT_MONTH_FROM_DATE(fdate);             \
    (rdate).day = FAT_DAY_FROM_DATE(fdate);                 \
}

#define FAT_TIME_TO_RTC_DATE(ftime, rdate)                  \
{                                                           \
    (rdate).hour = FAT_HOUR_FROM_TIME(ftime);               \
    (rdate).minute = FAT_MINUTE_FROM_TIME(ftime);           \
    (rdate).second = FAT_SECOND_FROM_TIME(ftime);           \
}

#define FAT_DATETIME_TO_RTC_DATETIME(fdate, ftime, rdate)   \
{                                                           \
    FAT_DATE_TO_RTC_DATE(fdate, rdate);                     \
    FAT_TIME_TO_RTC_DATE(ftime, rdate);                     \
}

#define FAT_DATETIME_TO_TIMESTAMP(fdate, ftime)             \
    __extension__ ({                                        \
        struct rtc_time tm;                                 \
        s32 ts;                                             \
        FAT_DATE_TO_RTC_DATE(fdate, tm);                    \
        FAT_TIME_TO_RTC_DATE(ftime, tm);                    \
        rtc_time_to_timestamp(&tm, &ts);                    \
        ts;                                                 \
    })

#define RTC_DATE_TO_FAT_DATE(rdate)                         \
    __extension__ ({                                        \
        (u16) (((rdate)->year - 1980) << 9) |               \
                ((rdate)->month << 5) |                     \
                ((rdate)->day);                             \
    })

#define RTC_DATE_TO_FAT_TIME(rdate)                         \
    __extension__ ({                                        \
        (u16) ((rdate)->hour << 11) |                       \
                ((rdate)->minute << 5) |                    \
                ((rdate)->second >> 1);                     \
    })


vfs_driver_t g_fat_ops;

#endif /* WITH_FS_FAT */
#endif
