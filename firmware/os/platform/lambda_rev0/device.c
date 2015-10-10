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
		ret = SUCCESS; //mc68681_init(d);
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

    /* Enumerate expansion cards */
    expansion_init();

    return SUCCESS;
}


/*
    expansion_init() - initialise expansion card slots
*/
void expansion_init()
{
    u32 irql;
    void *base_addr;
    u16 i;

	puts("Scanning expansion slots");

	for(base_addr = EXP_BASE_ADDR, irql = EXP_BASE_IRQ, i = 0; i < EXP_NUM_SLOTS;
        ++i, base_addr += EXP_ADDR_LEN, ++irql)
    {
        printf("slot %d (0x%08x-0x%08x irq %u): ", i, base_addr, base_addr + EXP_ADDR_LEN - 1,
               irql);

        if(EXP_PRESENT(i))
        {
            /* A card is present; read its identity from the first byte of its address space */
            u8 id;

            EXP_ID_ASSERT();        /* Assert EID line to ask peripherals to identify themselves */
            id = *((u8 *) EXP_BASE(i));
            EXP_ID_NEGATE();

            switch(id)
            {
                default:
                    printf("unknown peripheral %02x\n", id);
            }
        }
        else
            puts("vacant");
    }
}
