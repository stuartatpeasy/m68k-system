/*
    Network interface abstraction

    Part of ayumos


    (c) Stuart Wallace, May 2016.

    FIXME - replace printf() with kernel logging call of some sort, and remove #include stdio
*/

#include <kernel/device/device.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/net/interface.h>
#include <kernel/net/net.h>
#include <kernel/process.h>
#include <klibc/stdio.h>        // FIXME remove
#include <klibc/strings.h>


s32 net_interface_add(dev_t *dev);

net_iface_t *g_net_ifaces = NULL;

struct net_iface
{
    net_iface_t *       next;
    dev_t *             dev;            /* The hw device implementing this interface            */
    net_protocol_t      proto;          /* Interface protocol (e.g. np_ethernet -> Ethernet)    */
    net_address_t       hw_addr;        /* Hardware address                                     */
    net_address_t       proto_addr;     /* Protocol address                                     */
    net_iface_stats_t   stats;
};


/*
    net_interface_init() - detect and initialise network interfaces
*/
s32 net_interface_init()
{
    dev_t *dev = NULL;

    /* Iterate over hardware devices; look for any which identify as network interfaces. */
    while((dev = dev_get_next(dev)) != NULL)
        if(dev->type == DEV_TYPE_NET)
        {
            ks32 ret = net_interface_add(dev);

            if(ret != SUCCESS)
                printf("net: failed to add %s: %s\n", dev->name, kstrerror(ret));
        }

    return SUCCESS;
}


/*
    net_interface_add() - add a network interface based on the specified hardware device
*/
s32 net_interface_add(dev_t *dev)
{
    net_iface_t **p, *iface;
    net_addr_type_t addr_type;
    s32 ret;

    ret = dev->control(dev, dc_get_hw_addr_type, NULL, &addr_type);
    if(ret != SUCCESS)
        return ret;

    if(addr_type == na_ethernet)    /* Ethernet interface */
    {
        char buf[64];

        iface = (net_iface_t *) CHECKED_KMALLOC(sizeof(net_iface_t));
        iface->next         = NULL;
        iface->dev          = dev;
        iface->hw_addr.type = addr_type;
        iface->proto        = np_ethernet;

        bzero(&iface->stats, sizeof(net_iface_stats_t));

        ret = dev->control(dev, dc_get_hw_addr, NULL, &iface->hw_addr.addr);
        if(ret != SUCCESS)
        {
            kfree(iface);
            return ret;
        }

        for(p = &g_net_ifaces; *p != NULL; p = &(*p)->next)
            ;

        *p = iface;
        iface->proto_addr.type = na_unknown;

        net_address_print(&iface->hw_addr, buf, sizeof(buf));
        printf("net: added %s: %s\n", dev->name, buf);

        return proc_create(0, 0, "[net_rx]", NULL, net_receive, iface, 0, PROC_TYPE_KERNEL, NULL,
                           NULL);
    }
    else
        return EPROTONOSUPPORT;
}


/*
    net_get_iface_name() - get the device name associated with an interface
*/
const char *net_get_iface_name(const net_iface_t * const iface)
{
    return iface->dev->name;
}


/*
    net_interface_get_device() - return the device object associated with an interface
*/
dev_t *net_interface_get_device(net_iface_t * const iface)
{
    return iface->dev;
}


/*
    net_interface_get_proto_addr() - get the protocol address for an interface
*/
const net_address_t *net_interface_get_proto_addr(const net_iface_t * const iface)
{
    return &iface->proto_addr;
}


/*
    net_interface_get_hw_addr() - get the hardware address for an interface
*/
const net_address_t *net_interface_get_hw_addr(const net_iface_t * const iface)
{
    return &iface->hw_addr;
}


/*
    net_interface_set_proto_addr() - set the protocol address for an interface
*/
s32 net_interface_set_proto_addr(net_iface_t * const iface, const net_address_t * const addr)
{
    iface->proto_addr = *addr;
    return SUCCESS;
}


/*
    net_interface_hw_addr_broadcast() - obtain the broadcast hardware address associated with the
    specified interface.
*/
s32 net_interface_hw_addr_broadcast(net_iface_t * const iface, net_address_t * const addr)
{
    UNUSED(iface);
    UNUSED(addr);

    /* FIXME - implement this */
    puts("net_interface_hw_addr_broadcast() - not implemented");
    return EPROTONOSUPPORT;
}


/*
    net_interface_get_by_dev() - look up a network interface by device name.
*/
net_iface_t *net_interface_get_by_dev(const char * const name)
{
    net_iface_t **p;

    for(p = &g_net_ifaces; *p != NULL; p = &(*p)->next)
        if(!strcmp((*p)->dev->name, name))
            return *p;

    return NULL;
}


/*
    net_interface_get_proto() - get the network protocol implemented by a network interface
*/
net_protocol_t net_interface_get_proto(const net_iface_t * const iface)
{
    return iface->proto;
}


/*
    net_interface_stats_inc_cksum_err() - increment the "checksum error" count on an interface.
*/
void net_interface_stats_inc_cksum_err(net_iface_t * const iface)
{
    ++iface->stats.rx_checksum_err;
}


/*
    net_interface_stats_inc_dropped() - increment the "packets dropped" count on an interface.
*/
void net_interface_stats_inc_dropped(net_iface_t * const iface)
{
    ++iface->stats.rx_dropped;
}
