#ifndef _MBR_H_
#define _MBR_H_
/*
	Master boot record (MBR) handling functions and type definitions

	Stuart Wallace, July 2012.
*/


#include "include/defs.h"
#include "include/types.h"

#define MBR_SIGNATURE		(0xaa55)		/* validation signature in bytes 510-511 of a MBR	*/
#define MBR_CODE_AREA_LEN	(440)			/* Size of the boot code area at the start of a MBR	*/
#define MBR_NUM_PARTITIONS	(4)				/* Number of partition records in a MBR				*/


/* Partition status field values */
enum mbr_partition_status
{
	MBR_PS_NOT_BOOTABLE = 0x00,
	MBR_PS_BOOTABLE = 0x80
} __attribute__((packed));


/* A cylinder/head/sector address */
struct chs_address
{
	u8 head;
	u8 cyl_sector;
	u8 cylinder;
} __attribute__((aligned (1)));


/* A MBR partition record */
struct mbr_partition
{
	enum mbr_partition_status status;	/* bootable/non-bootable flag */
	struct chs_address first_sector;
	u8 type;
	struct chs_address last_sector;
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


int mbr_read();

#endif

