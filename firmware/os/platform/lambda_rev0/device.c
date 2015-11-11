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
#include <device/auto.h>
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

    /*
        MC68681 DUART

        The parent device (duart0) and the first serial port (ser0) are initialised by
        plat_console_init().  Only the second serial port (ser1) is initialised here.
    */

    /* Child device: serial channel B */
    dev_register(DEV_TYPE_SERIAL, DEV_SUBTYPE_NONE, "ser", IRQL_NONE, LAMBDA_MC68681_BASE,
                 &dev, "MC68681 serial port B", g_lambda_duart, mc68681_serial_b_init);


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
    expansion_init() - identify and initialise devices in expansion card slots
*/
void expansion_init()
{
    u16 i;
    void *base_addr;
    u32 irql;

	for(base_addr = EXP_BASE_ADDR, irql = EXP_BASE_IRQ, i = 0; i < EXP_NUM_SLOTS;
        ++i, base_addr += EXP_ADDR_LEN, ++irql)
    {
        if(!(mc68681_read_ip(g_lambda_console) & EXP_PD_MASK(i)))
        {
            /* A card is present; read its identity from the first byte of its address space */
            u8 id;
            dev_t *dev;
            s32 ret;

            /* Assert nEID to ask peripherals to identify themselves */
            mc68681_reset_op_bits(g_lambda_console, BIT(EXP_ID));

            id = *((u8 *) EXP_BASE(i));

            /* Negate nEID */
            mc68681_set_op_bits(g_lambda_console, BIT(EXP_ID));


            printf("slot %d (%x-%x, irq %u): ", i, (u32) base_addr,
                    (u32) base_addr + EXP_ADDR_LEN - 1, irql);

            ret = dev_auto_init(id, base_addr, irql, NULL, &dev);

            if(ret == SUCCESS)
                puts(dev->human_name);
            else if(ret == ENOENT)
                printf("unknown peripheral (hardware ID %02x)\n", id);
        }
    }
}
