/*
	boot.c: declarations of functions useful during boot

	Part of the as-yet-unnamed MC68010 operating system


	(c) Stuart Wallace, 17th October 2015.
*/

#include <kernel/include/boot.h>
#include <kernel/device/devctl.h>
#include <kernel/include/platform.h>
#include <kernel/util/kutil.h>
#include <klibc/include/stdio.h>


const char * const unknown = "(unknown)";


/*
    boot_early_fail() - report an "early failure" by flashing the motherboard LEDs.

    This code assumes that interrupts have not yet been enabled.  This function does not return.
*/
void boot_early_fail(ku32 code)
{
    /* TODO - remove hardwired loops */
    while(1)
    {
        u32 i, j;

        for(j = 250000; j; j--)
            plat_led_off(LED_ALL);

        for(i = 0; i < code; ++i)
        {
            u32 j;

            plat_led_on(LED_RED);
            for(j = 100000; j; j--)
                ;

            plat_led_off(LED_ALL);
            for(j = 100000; j; j--)
                ;
        }
    }
}


/*
    boot_list_mass_storage() - dump a list of mass-storage devices to the console
*/
void boot_list_mass_storage()
{
    dev_t *dev = NULL;

    while((dev = dev_get_next(dev)) != NULL)
    {
        if((dev->type == DEV_TYPE_BLOCK) && (dev->subtype == DEV_SUBTYPE_MASS_STORAGE))
        {
            const char *model = unknown;
            const char *serial = unknown;
            const char *firmware = unknown;

            dev->control(dev, dc_get_model, NULL, &model);
            dev->control(dev, dc_get_serial, NULL, &serial);
            dev->control(dev, dc_get_firmware_ver, NULL, &firmware);

            printf("%s: %s, %uMB [serial %s firmware %s]\n", dev->name, model,
                   dev->len >> (20 - log2(dev->block_size)), serial, firmware);
        }
    }
}


/*
    boot_list_partitions() - dump a list of partitions to the console
*/
void boot_list_partitions()
{
    dev_t *dev = NULL;

    while((dev = dev_get_next(dev)) != NULL)
    {
        if((dev->type == DEV_TYPE_BLOCK) && (dev->subtype == DEV_SUBTYPE_PARTITION))
        {
            const char *active = unknown;
            const char *bootable = unknown;
            const char *type = unknown;

            u32 is_active = 0, is_bootable = 0;

            if(dev->control(dev, dc_get_partition_active, NULL, &is_active) == SUCCESS)
                active = is_active ? "active" : "inactive";

            if(dev->control(dev, dc_get_bootable, NULL, &is_bootable) == SUCCESS)
                bootable = is_bootable ? ", bootable" : "";

            dev->control(dev, dc_get_partition_type_name, NULL, &type);

            printf("%s: %4uMB [%s, %s%s]\n", dev->name, dev->len >> (20 - log2(dev->block_size)),
                   type, active, bootable);
        }
    }
}
