/*
    Hardware device enumeration for the "lambda" (MC68010) motherboard

    Stuart Wallace, September 2015
*/

#include <platform/platform.h>

#include <stdio.h>
#include <string.h>
#include <kutil/kutil.h>
#include <memory/kmalloc.h>
#include <platform/lambda_rev0/device.h>
#include <device/mc68681.h>                 /* DUART            */
#include <device/ds17485.h>                 /* RTC              */
#include <platform/lambda_rev0/ata.h>       /* ATA interface    */
#include <platform/lambda_rev0/exp16.h>     /* Expansion slots  */

s32 plat_dev_enumerate(dev_t *root_dev)
{
	dev_t *d;
	s32 ret;

    /*
        Enumerate on-board devices (address range 0xe000000-0xefffff)
    */

    /* MC68681 DUART */
	ret = dev_create(DEV_TYPE_SERIAL, DEV_SUBTYPE_NONE, "ser", 27, (void *) 0xe00000, &d);
	if(ret == SUCCESS)
	{
		ret = mc68681_init(d);
		if(ret == SUCCESS)
			dev_add_child(root_dev, d);
		else
		{
			kfree(d);
			printf("mc68681: init failed: %s\n", kstrerror(ret));
		}
	}
	else
		printf("mc68681: device creation failed: %s\n", kstrerror(ret));

    /* ATA interface */
	ret = dev_create(DEV_TYPE_BLOCK, DEV_SUBTYPE_MASS_STORAGE, "ata", 26, (void *) 0xe20000, &d);
	if(ret == SUCCESS)
	{

	}
	else
		printf("ata: device creation failed: %s\n", kstrerror(ret));

    /* DS17485 RTC */

    /* Memory device */

    /*
        Enumerate expansion cards (if any)
    */

    return SUCCESS;
}
