/*
    Network protocol abstraction

    Part of ayumos


    (c) Stuart Wallace, May 2016.

    FIXME: remove debug printf() and #include of stdio
*/

#include <kernel/net/protocol.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/net/arp.h>
#include <kernel/net/ipv4.h>
#include <kernel/net/ethernet.h>
#include <klibc/stdio.h>


s32 net_protocol_register_driver(net_protocol_init_fn_t init_fn);
s32 net_rx_unimplemented(net_packet_t *packet);
s32 net_tx_unimplemented(const net_address_t *src, const net_address_t *dest, net_packet_t *packet);
s32 net_packet_alloc_unimplemented(net_iface_t *iface, ku32 len, net_packet_t **packet);

net_proto_driver_t *g_net_proto_drivers;

net_protocol_init_fn_t g_net_proto_init_fns[] =
{
    arp_init,
    ipv4_init,
    eth_init
};


/*
    net_protocol_init() - initialise protocol drivers
*/
s32 net_protocol_init()
{
    u32 i;

    for(i = 0; i < sizeof(g_net_proto_init_fns); ++i)
    {
        ks32 ret = net_protocol_register_driver(g_net_proto_init_fns[i]);
        if(ret != SUCCESS)
            return ret;
    }

    return SUCCESS;
}


/*
    net_protocol_register_driver() - register a driver for a particular network protocol
*/
//s32 net_protocol_register_driver(s32 (*init_fn)(net_proto_driver_t *))
s32 net_protocol_register_driver(net_protocol_init_fn_t init_fn)
{
    s32 ret;
    net_proto_driver_t *driver = (net_proto_driver_t *) CHECKED_KMALLOC(sizeof(net_proto_driver_t)),
                        *p;

    driver->rx              = net_rx_unimplemented;
    driver->tx              = net_tx_unimplemented;

    driver->next = NULL;

    ret = init_fn(driver);
    if(ret != SUCCESS)
    {
        kfree(driver);
        return ret;
    }

    if(g_net_proto_drivers)
    {
        for(p = g_net_proto_drivers; p->next; p = p->next)
            ;

        p->next = driver;
    }
    else
        g_net_proto_drivers = driver;

    printf("net: registered protocol %s\n", driver->name);
    return SUCCESS;
}


/*
    net_rx_unimplemented() - default handler for <interface>_rx()
*/
s32 net_rx_unimplemented(net_packet_t *packet)
{
    UNUSED(packet);
    return ENOSYS;
}


/*
    net_tx_unimplemented() - default handler for <interface>_tx()
*/
s32 net_tx_unimplemented(const net_address_t *src, const net_address_t *dest, net_packet_t *packet)
{
    UNUSED(src);
    UNUSED(dest);
    UNUSED(packet);
    return ENOSYS;
}


/*
    net_packet_alloc_unimplemented() - default handler for <interface>_packet_alloc()
    FIXME - maybe remove this?  I don't think it's used any longer
*/
s32 net_packet_alloc_unimplemented(net_iface_t *iface, ku32 len, net_packet_t **packet)
{
    UNUSED(iface);
    UNUSED(len);
    UNUSED(packet);
    return ENOSYS;
}


/*
    net_protocol_get_driver() - look up a protocol driver by protocol
*/
net_proto_driver_t *net_protocol_get_driver(const net_protocol_t proto)
{
    net_proto_driver_t *d;

    for(d = g_net_proto_drivers; d && (d->proto != proto); d = d->next)
        ;

    return d;
}
