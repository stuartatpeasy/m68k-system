/*
    Network interface abstraction

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <device/device.h>
#include <kernel/net/ethernet.h>
#include <kernel/net/net.h>
#include <stdio.h>


s32 net_add_interface(dev_t *dev);

net_iface_t *g_net_ifaces = NULL;


/*
    net_init() - initialise networking layer
*/
s32 net_init()
{
    dev_t *dev = NULL;

    /* Iterate over hardware devices; look for any which identify as network interfaces. */
    while((dev = dev_get_next(dev)) != NULL)
    {
        if(dev->type == DEV_TYPE_NET)
        {
            if(dev->subtype == DEV_SUBTYPE_ETHERNET)
            {
                ks32 ret = net_add_interface(dev);

                if(ret != SUCCESS)
                    printf("net: failed to add %s: %s\n", dev->name, kstrerror(ret));
            }
            else
            {
                printf("net: ignoring unsupported network device %s\n", dev->name);
            }
        }
    }

    return SUCCESS;
}


/*
    net_add_interface() - add a network interface based on the specified hardware device
*/
s32 net_add_interface(dev_t *dev)
{
    net_iface_t **p, *iface;
    mac_addr_t *ma;
    s32 ret;

    iface = (net_iface_t *) CHECKED_KCALLOC(1, sizeof(net_iface_t));
    iface->dev = dev;

    ret = dev->control(dev, dc_get_hw_addr_type, NULL, &iface->hw_addr_type);
    if(ret != SUCCESS)
    {
        kfree(iface);
        return ret;
    }

    if(iface->hw_addr_type != na_ethernet)
    {
        kfree(iface);
        return EPROTONOSUPPORT;
    }

    ret = dev->control(dev, dc_get_hw_addr, NULL, &iface->hw_addr);
    if(ret != SUCCESS)
    {
        kfree(iface);
        return ret;
    }

    for(p = &g_net_ifaces; *p != NULL; p = &(*p)->next)
        ;

    *p = iface;

    ma = (mac_addr_t *) &iface->hw_addr;
    printf("net: added %s: %02x:%02x:%02x:%02x:%02x:%02x\n", dev->name,
           ma->b[0], ma->b[1], ma->b[2], ma->b[3], ma->b[4], ma->b[5]);

    return SUCCESS;
}
