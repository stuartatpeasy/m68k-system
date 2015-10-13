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
#include <device/block/ata/ata.h>           /* ATA interface    */


dev_t *g_lambda_duart;      /* DUART device - stored separately for early console init  */
dev_t *g_lambda_console;    /* Console device - stored separately for early init        */


/*
    plat_dev_enumerate() - enumerate built-in peripherals.
    On this board, built-in peripherals occupy the memory range e00000-efffff.
*/
s32 plat_dev_enumerate(dev_t *root_dev)
{
	dev_t *d;
	s32 ret;

    /*
        MC68681 DUART
    */
    /* Main DEV_TYPE_MULTI device, representing the whole chip */
    ret = SUCCESS;
    if(g_lambda_duart == NULL)
    {
        ret = dev_create(DEV_TYPE_MULTI, DEV_SUBTYPE_NONE, "duart", 0, (void *) 0xe00000,
                            &g_lambda_duart);
        if(ret == SUCCESS)
            ret = mc68681_init(g_lambda_duart);
    }

    if(ret == SUCCESS)
        dev_add_child(root_dev, g_lambda_duart);

    /* Child device: serial channel A */
    ret = SUCCESS;
    if(g_lambda_console == NULL)
    {
        ret = dev_create(DEV_TYPE_SERIAL, DEV_SUBTYPE_NONE, "ser", 27, (void *) 0xe00000,
                            &g_lambda_console);
        if(ret == SUCCESS)
            ret = mc68681_serial_a_init(g_lambda_console);
    }

    if(ret == SUCCESS)
        dev_add_child(g_lambda_duart, g_lambda_console);

    /* Child device: serial channel B */
    ret = dev_create(DEV_TYPE_SERIAL, DEV_SUBTYPE_NONE, "ser", 27, (void *) 0xe00000,
                        &d);
    if(ret == SUCCESS)
    {
        ret = mc68681_serial_b_init(d);
        if(ret == SUCCESS)
            dev_add_child(g_lambda_duart, d);
    }


    /* DS17485 RTC */
    ret = dev_create(DEV_TYPE_RTC, DEV_SUBTYPE_NONE, "rtc", IRQL_NONE, (void *) 0xe10000, &d);
    if(ret == SUCCESS)
    {
        ret = ds17485_init(d);
        if(ret == SUCCESS)
            dev_add_child(root_dev, d);
        else
            printf("rtc: DS17485 init failed: %s\n", kstrerror(ret));
    }
    else
        printf("rtc: DS17485 device creation failed: %s\n", kstrerror(ret));


    /* ATA interface */
	ret = dev_create(DEV_TYPE_BLOCK, DEV_SUBTYPE_MASS_STORAGE, "ata", 26, (void *) 0xe20000, &d);
	if(ret == SUCCESS)
	{

	}
	else
		printf("ata: device creation failed: %s\n", kstrerror(ret));

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

        if(!(mc68681_read_ip(g_lambda_console) & EXP_PD_MASK(i)))
        {
            /* A card is present; read its identity from the first byte of its address space */
            u8 id;

            /* Assert nEID to ask peripherals to identify themselves */
            mc68681_reset_op_bits(g_lambda_console, BIT(EXP_ID));

            id = *((u8 *) EXP_BASE(i));

            /* Negate nEID */
            mc68681_set_op_bits(g_lambda_console, BIT(EXP_ID));

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
