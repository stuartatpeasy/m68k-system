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
    /* DEV_TYPE_MULTI device representing the whole chip */
    ret = SUCCESS;
    if(g_lambda_duart == NULL)
    {
        ret = dev_create(DEV_TYPE_MULTI, DEV_SUBTYPE_NONE, "duart", IRQL_NONE, (void *) 0xe00000,
                            &g_lambda_duart);
        if(ret == SUCCESS)
            ret = mc68681_init(g_lambda_duart);
        else
            kfree(g_lambda_duart);
    }

    if(ret == SUCCESS)
    {
        dev_add_child(root_dev, g_lambda_duart);

        /* Child device: serial channel A */
        ret = SUCCESS;
        if(g_lambda_console == NULL)
        {
            ret = dev_create(DEV_TYPE_SERIAL, DEV_SUBTYPE_NONE, "ser", 27, (void *) 0xe00000,
                                &g_lambda_console);
            if(ret == SUCCESS)
                ret = mc68681_serial_a_init(g_lambda_console);
            else
                kfree(g_lambda_console);
        }

        if(ret == SUCCESS)
            dev_add_child(g_lambda_duart, g_lambda_console);

        /* Child device: serial channel B */
        ret = dev_create(DEV_TYPE_SERIAL, DEV_SUBTYPE_NONE, "ser", 27, (void *) 0xe00000, &d);
        if(ret == SUCCESS)
        {
            ret = mc68681_serial_b_init(d);
            if(ret == SUCCESS)
                dev_add_child(g_lambda_duart, d);
        }
    }


    /*
        DS17485 RTC
    */
    /* DEV_TYPE_MULTI device representing the whole chip */
    if(dev_register(DEV_TYPE_MULTI, DEV_SUBTYPE_NONE, "nvrtc", IRQL_NONE, (void *) 0xe10000, &d,
                        "DS17485", root_dev, ds17485_init) == SUCCESS)
    {
        dev_t *sub_dev;

        /* Child device: RTC */
        dev_register(DEV_TYPE_RTC, DEV_SUBTYPE_NONE, "rtc", IRQL_NONE, (void *) 0xe10000, &sub_dev,
                        "DS17485 RTC", d, ds17485_rtc_init);

        /* Child device: NVRAM */
        dev_register(DEV_TYPE_NVRAM, DEV_SUBTYPE_NONE, "nvram", IRQL_NONE, (void *) 0xe10000,
                        &sub_dev, "DS17485 NVRAM", d, ds17485_nvram_init);
    }


    /*
        ATA interface
    */
#if 0
    /* DEV_TYPE_MULTI device representing the whole interface */
    if(dev_register(DEV_TYPE_MULTI, DEV_SUBTYPE_NONE, "ataif", IRQL_NONE, (void *) 0xe20000, &d,
                        "ATA interface", root_dev, ata_init) == SUCCESS)
    {
        dev_t *sub_dev;

        /* Child device: primary ATA channel */
        dev_register(DEV_TYPE_BLOCK, DEV_SUBTYPE_MASS_STORAGE, "ata", IRQL_NONE, (void *) 0xe20000,
                        &sub_dev, "ATA channel 0", d, ata_channel_0_init);

        /* Child device: secondary ATA channel */
        dev_register(DEV_TYPE_BLOCK, DEV_SUBTYPE_MASS_STORAGE, "ata", IRQL_NONE, (void *) 0xe20000,
                        &sub_dev, "ATA channel 1", d, ata_channel_1_init);
    }
#endif


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
