#ifndef INCLUDE_MBR_H_INC
#define INCLUDE_MBR_H_INC
/*
	Declarations relating to master boot records (MBR)

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace <stuartw@atom.net>, July 2012.
*/


#include <kernel/include/defs.h>
#include <kernel/include/types.h>


#define MBR_SECTOR_LEN		(512)
#define MBR_SIGNATURE		(0xaa55)		/* validation signature in bytes 510-511 of a MBR	*/
#define MBR_CODE_AREA_LEN	(440)			/* Size of the boot code area at the start of a MBR	*/
#define MBR_NUM_PARTITIONS	(4)				/* Number of partition records in a MBR				*/


/* Partition status field values */
enum mbr_partition_status
{
	MBR_PS_NOT_BOOTABLE = 0x00,
	MBR_PS_BOOTABLE = 0x80
} __attribute__((packed));


/* A MBR partition record */
struct mbr_partition
{
	enum mbr_partition_status status;	/* bootable/non-bootable flag */
	u8 chs_first_sector_head;
	u8 chs_first_sector_cyl_sector;
	u8 chs_first_sector_cylinder;
	u8 type;
	u8 chs_last_sector_head;
	u8 chs_last_sector_cyl_sector;
	u8 chs_last_sector_cylinder;
	u32 first_sector_lba;
	u32 num_sectors;
} __attribute__((packed));


/* A master boot record */
struct mbr
{
	u8 code[MBR_CODE_AREA_LEN];
	u32 disk_signature;
	u16 reserved;
	struct mbr_partition partition[MBR_NUM_PARTITIONS];
	u16 mbr_signature;
} __attribute__((packed));


#endif
