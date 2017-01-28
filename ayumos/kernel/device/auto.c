/*
	Automatic device init and driver selection, based on hardware device ID

	Part of ayumos


	(c) Stuart Wallace <stuartw@atom.net>, October 2015.
*/

#include <kernel/include/device/auto.h>

/* Include the files containing drivers appearing in driver_map[], below */
#include <driver/encx24j600.h>
#include <driver/ps2controller.h>


typedef struct driver_map_entry
{
    u8              hw_id;
    dev_type_t      type;
    dev_subtype_t   subtype;
    const char *    name;
    const char *    human_name;
    driver_init_fn  init_fn;
} driver_map_entry_t;


const driver_map_entry_t driver_map[] =
{
#ifdef WITH_DRV_ENCX24J600
    {
        .hw_id      = 0x81,
        .type       = DEV_TYPE_NET,
        .subtype    = DEV_SUBTYPE_ETHERNET,
        .name       = "eth",
        .human_name = "ENCx24J600 Ethernet controller",
        .init_fn    = encx24j600_init
    },
#endif /* WITH_DRV_ENCX24J600 */

#ifdef WITH_DRV_PS2CONTROLLER
    {
        .hw_id      = 0x82,
        .type       = DEV_TYPE_MULTI,
        .subtype    = DEV_SUBTYPE_NONE,
        .name       = "ps2",
        .human_name = "PS/2 port controller",
        .init_fn    = ps2controller_init
    }
#endif /* WITH_DRV_PS2CONTROLLER */
};


/*
    dev_auto_init() - initialise a device based on its hardware ID
*/
s32 dev_auto_init(ku8 hw_id, void *base_addr, ku32 irql, dev_t *parent_dev, dev_t **dev)
{
    const driver_map_entry_t *p;
    UNUSED(parent_dev);

    FOR_EACH(p, driver_map)
        if(p->hw_id == hw_id)
        {
            s32 ret;
            dev_t *d;

            ret = dev_create(p->type, p->subtype, p->name, irql, base_addr, &d, p->human_name,
                             NULL, p->init_fn);

            if((ret == SUCCESS) && (dev != NULL))
                *dev = d;

            return ret;
        }

    return ENOENT;
}
