/*
    Hardware device enumeration for the "lambda" (MC68010) motherboard

    Stuart Wallace, September 2015
*/

#include <kernel/include/platform.h>

#include <klibc/include/stdio.h>
#include <klibc/include/string.h>
#include <kernel/device/ata.h>              /* ATA interface    						*/
#include <kernel/device/auto.h>				/* Automatic peripheral driver selection	*/
#include <kernel/include/memory/kmalloc.h>
#include <kernel/util/kutil.h>
#include <platform/lambda/include/device.h>
#include <driver/mc68681.h>                 /* DUART            						*/
#include <driver/ds17485.h>                 /* RTC              						*/

/*
    Interrupt assignments

    IRQ         irql          rev0                      rev1
    --------------------------------------------------------------------
    1           25            timer (MC68681)           timer (MC68681)
    2           26            ATA                       ATA
    3           27            DUART                     DUART
    4           28            expansion 0               RTC
    5           29            expansion 1               expansion 0 & 1
    6           30            expansion 2               expansion 2 & 3
    7 (NMI)     31            expansion 3               NMI button
    --------------------------------------------------------------------
*/

dev_t *g_lambda_duart;      /* DUART device - stored separately for early console init  */
dev_t *g_lambda_console;    /* Console device - stored separately for early init        */


/*
    plat_dev_enumerate() - enumerate built-in peripherals.
    On this board, built-in peripherals occupy the memory range e00000-efffff.
*/
s32 plat_dev_enumerate()
{
	dev_t *dev;

    /*
        MC68681 DUART
    */
    if(dev_create(DEV_TYPE_MULTI, DEV_SUBTYPE_NONE, "duart", LAMBDA_MC68681_IRQL,
                  LAMBDA_MC68681_BASE, &dev, "MC68681 DUART", NULL, mc68681_init) == SUCCESS)
    {
        g_lambda_duart = dev;

        /* Child device: serial channel A */
        dev_create(DEV_TYPE_SERIAL, DEV_SUBTYPE_NONE, "ser", IRQL_NONE, LAMBDA_MC68681_BASE,
                   &g_lambda_console, "MC68681 serial port A", dev, mc68681_serial_a_init);

        /* Child device: serial channel B */
        dev_create(DEV_TYPE_SERIAL, DEV_SUBTYPE_NONE, "ser", IRQL_NONE, LAMBDA_MC68681_BASE,
                   NULL, "MC68681 serial port B", dev, mc68681_serial_b_init);

        /*
            Set the MC68681 general-purpose outputs to safe initial values:
                OP7     nLEDRED     0
                OP6     nLEDGREEN   0
                OP5     BUZZER      0       doesn't matter; will be changed in the next stmt
                OP4     n/c         0
                OP3     nTIMERIRQ   1       negate timer IRQ
                OP2     nEID        1       negate nEID
                OP1     RTS_B       0       negate RTS_B
                OP0     RTS_A       0       negate RTS_A
        */
        mc68681_reset_op_bits(g_lambda_duart, 0xf3);
        mc68681_set_op_bits(g_lambda_duart, 0x0c);

        /*
            Switch off the beeper.  In hardware rev0, the beeper is an active-high output; in
            subsequent revisions it's active-low.
        */
#if (PLATFORM_REV == 0)
		mc68681_reset_op_bits(g_lambda_duart, BIT(LAMBDA_DUART_BEEPER_OUTPUT));
#else
		mc68681_set_op_bits(g_lambda_duart, BIT(LAMBDA_DUART_BEEPER_OUTPUT));
#endif
    }


    /*
        DS17485 RTC
    */
    /* DEV_TYPE_MULTI device representing the whole chip */
    if(dev_create(DEV_TYPE_MULTI, DEV_SUBTYPE_NONE, "nvrtc", IRQL_NONE,
                  LAMBDA_DS17485_BASE, &dev, "DS17485 RTC/NVRAM IC", NULL, ds17485_init) == SUCCESS)
    {
        /* Child device: RTC */
        dev_create(DEV_TYPE_RTC, DEV_SUBTYPE_NONE, "rtc", LAMBDA_DS17485_IRQL, LAMBDA_DS17485_BASE,
                   NULL, "DS17485 RTC", dev, ds17485_rtc_init);

        /* Child device: user NVRAM */
        dev_create(DEV_TYPE_NVRAM, DEV_SUBTYPE_NONE, "nvram", IRQL_NONE, LAMBDA_DS17485_BASE, NULL,
                   "DS17485 user NVRAM", dev, ds17485_user_ram_init);

        /* Child device: extended NVRAM */
        dev_create(DEV_TYPE_NVRAM, DEV_SUBTYPE_NONE, "nvram", IRQL_NONE, LAMBDA_DS17485_BASE, NULL,
                   "DS17485 extended NVRAM", dev, ds17485_ext_ram_init);

        /* Child device: periodic interrupt generator */
        dev_create(DEV_TYPE_TIMER, DEV_SUBTYPE_NONE, "timer", IRQL_NONE, LAMBDA_DS17485_BASE, NULL,
                   "DS17485 periodic interrupt", dev, ds17485_timer_init);
    }


    /*
        ATA interface
    */
    /* DEV_TYPE_MULTI device representing the whole interface */
    if(dev_create(DEV_TYPE_MULTI, DEV_SUBTYPE_NONE, "ataif", LAMBDA_ATA_IRQL, LAMBDA_ATA_BASE,
                  &dev, "ATA interface", NULL, ata_init) == SUCCESS)
    {
        /* Child device: primary ATA channel */
        dev_create(DEV_TYPE_BLOCK, DEV_SUBTYPE_MASS_STORAGE, "ata", IRQL_NONE,
                   LAMBDA_ATA_BASE, NULL, "ATA channel 0", dev, ata_master_init);

        /* Child device: secondary ATA channel */
        dev_create(DEV_TYPE_BLOCK, DEV_SUBTYPE_MASS_STORAGE, "ata", IRQL_NONE,
                   LAMBDA_ATA_BASE, NULL, "ATA channel 1", dev, ata_slave_init);
    }

    /* Memory device */

    /* Enumerate expansion cards */
    expansion_init();

    return SUCCESS;
}


/*
    expansion_init() - identify and initialise devices in expansion card slots

    FIXME: bus error if a peripheral asserts nPD but does not respond to an ID request.
    Use an alternative bus error handler when running the ID request cycle.
*/
void expansion_init()
{
    u16 i;
    void *base_addr;

	for(base_addr = LAMBDA_EXP_BASE_ADDR, i = 0; i < LAMBDA_EXP_NUM_SLOTS;
        ++i, base_addr += LAMBDA_EXP_ADDR_LEN)
    {
        if(!(mc68681_read_ip(g_lambda_console) & LAMBDA_EXP_PD_MASK(i)))
        {
            /* A card is present; read its identity from the first byte of its address space */
            u8 id;
            dev_t *dev;
            s32 ret;

            /* Assert nEID to ask peripherals to identify themselves */
            mc68681_reset_op_bits(g_lambda_console, BIT(LAMBDA_EXP_ID));

            /* Read device ID byte */
            id = *((u8 *) LAMBDA_EXP_BASE(i));

            /* Negate nEID */
            mc68681_set_op_bits(g_lambda_console, BIT(LAMBDA_EXP_ID));

            printf("slot %d (%x-%x, irq %u): ", i, (u32) base_addr,
                    (u32) base_addr + LAMBDA_EXP_ADDR_LEN - 1, LAMBDA_EXP_IRQ(i));

            ret = dev_auto_init(id, base_addr, LAMBDA_EXP_IRQ(i), NULL, &dev);

            if(ret == SUCCESS)
                puts(dev->human_name);
            else if(ret == ENOENT)
                printf("unknown peripheral (hardware ID %02x)\n", id);
        }
    }
}
