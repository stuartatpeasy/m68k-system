/*
	Master boot record parser

	Stuart Wallace, July 2012.
*/


#include <stdio.h>			/* XXX for testing */
#include "sector_read.h"	/* XXX for testing */

#include "include/byteorder.h"
#include "mbr.h"


/*
	Read the master boot record from a device.

	CHS addressing is not supported.  LBA must be used.
*/
int mbr_read()
{
	struct mbr m;
	int i;
	
	if(read_sector(0, &m))
		return FAIL;

	if(LE2H16(m.mbr_signature) != MBR_SIGNATURE)
	{
		/* Invalid MBR */
		return FAIL;
	}

	for(i = 0; i < 4; i++)
	{
		const struct mbr_partition * const p = &m.partition[i];

		printf("%d: Type 0x%02x  LBA extent %u-%u (%u sectors, %uMB)%s\n", i,
				p->type, p->first_sector_lba, p->first_sector_lba + p->num_sectors,
				p->num_sectors, p->num_sectors >> 11,
				(p->status == MBR_PS_BOOTABLE) ? " bootable" : "");
	}

	return SUCCESS;
}

