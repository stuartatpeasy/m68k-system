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
#include <device/ata.h>                     /* ATA interface    */


dev_t *g_lambda_duart;      /* DUART device - stored separately for early console init  */
dev_t *g_lambda_console;    /* Console device - stored separately for early init        */


/*
    plat_dev_enumerate() - enumerate built-in peripherals.
    On this board, built-in peripherals occupy the memory range e00000-efffff.
*/
s32 plat_dev_enumerate()
{
	dev_t *dev, *sub_dev;
	s32 ret;

    /*
        MC68681 DUART
    */
    /* DEV_TYPE_MULTI device representing the whole chip */
    ret = SUCCESS;
    if(g_lambda_duart == NULL)
    {
        ret = dev_create(DEV_TYPE_MULTI, DEV_SUBTYPE_NONE, "duart", LAMBDA_MC68681_IRQL,
                         LAMBDA_MC68681_BASE, &g_lambda_duart);
        if(ret == SUCCESS)
            ret = mc68681_init(g_lambda_duart);
        else
            kfree(g_lambda_duart);
    }

    if(ret == SUCCESS)
    {
        dev_add_child(NULL, g_lambda_duart);

        /* Child device: serial channel A */
        ret = SUCCESS;
        if(g_lambda_console == NULL)
        {
            ret = dev_create(DEV_TYPE_SERIAL, DEV_SUBTYPE_NONE, "ser", LAMBDA_MC68681_IRQL,
                             LAMBDA_MC68681_BASE, &g_lambda_console);
            if(ret == SUCCESS)
                ret = mc68681_serial_a_init(g_lambda_console);
            else
                kfree(g_lambda_console);
        }

        if(ret == SUCCESS)
            dev_add_child(g_lambda_duart, g_lambda_console);

        /* Child device: serial channel B */
        ret = dev_create(DEV_TYPE_SERIAL, DEV_SUBTYPE_NONE, "ser", LAMBDA_MC68681_IRQL,
                         LAMBDA_MC68681_BASE, &dev);
        if(ret == SUCCESS)
        {
            ret = mc68681_serial_b_init(dev);
            if(ret == SUCCESS)
                dev_add_child(g_lambda_duart, dev);
        }
    }


    /*
        DS17485 RTC
    */
    /* DEV_TYPE_MULTI device representing the whole chip */
    if(dev_register(DEV_TYPE_MULTI, DEV_SUBTYPE_NONE, "nvrtc", IRQL_NONE,
                    LAMBDA_DS17485_BASE, &dev, "DS17485", NULL, ds17485_init) == SUCCESS)
    {
        /* Child device: RTC */
        dev_register(DEV_TYPE_RTC, DEV_SUBTYPE_NONE, "rtc", LAMBDA_DS17485_IRQL,
                     LAMBDA_DS17485_BASE, &sub_dev, "DS17485 RTC", dev, ds17485_rtc_init);

        /* Child device: user NVRAM */
        dev_register(DEV_TYPE_NVRAM, DEV_SUBTYPE_NONE, "nvram", IRQL_NONE,
                     LAMBDA_DS17485_BASE, &sub_dev, "DS17485 user NVRAM", dev,
                     ds17485_user_ram_init);

        /* Child device: extendd NVRAM */
        dev_register(DEV_TYPE_NVRAM, DEV_SUBTYPE_NONE, "nvram", IRQL_NONE,
                     LAMBDA_DS17485_BASE, &sub_dev, "DS17485 extended NVRAM", dev,
                     ds17485_ext_ram_init);
    }


    /*
        ATA interface
    */
    /* DEV_TYPE_MULTI device representing the whole interface */
    if(dev_register(DEV_TYPE_MULTI, DEV_SUBTYPE_NONE, "ataif", LAMBDA_ATA_IRQL, LAMBDA_ATA_BASE,
                    &dev, "ATA interface", NULL, ata_init) == SUCCESS)
    {
        /* Child device: primary ATA channel */
        dev_register(DEV_TYPE_BLOCK, DEV_SUBTYPE_MASS_STORAGE, "ata", IRQL_NONE,
                     LAMBDA_ATA_BASE, &sub_dev, "ATA channel 0", dev, ata_master_init);

        /* Child device: secondary ATA channel */
        dev_register(DEV_TYPE_BLOCK, DEV_SUBTYPE_MASS_STORAGE, "ata", IRQL_NONE,
                     LAMBDA_ATA_BASE, &sub_dev, "ATA channel 1", dev, ata_slave_init);
    }

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
