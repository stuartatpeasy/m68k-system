/*
    Network protocol abstraction

    Part of ayumos


    (c) Stuart Wallace, May 2016.

    FIXME: remove debug printf() and #include of stdio
*/

#include <kernel/net/protocol.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/net/packet.h>
#include <klibc/stdio.h>
#include <klibc/string.h>


s32 net_rx_unimplemented(net_packet_t *packet);
s32 net_tx_unimplemented(const net_address_t *src, const net_address_t *dest, net_packet_t *packet);
s32 net_packet_alloc_unimplemented(const net_address_t * const addr, ku32 len,
                                   net_iface_t * const iface, net_packet_t **packet);
s32 net_addr_compare_unimplemented(const net_address_t * const a1, const net_address_t * const a2);

net_proto_driver_t *g_net_proto_drivers;



/* Network protocol driver */
struct net_proto_driver
{
    net_protocol_t          proto;
    const char *            name;
    net_rx_fn               rx;
    net_tx_fn               tx;
    net_addr_compare_fn     addr_compare;
    net_packet_alloc_fn     packet_alloc;
    net_proto_driver_t *    next;
};


/*
    net_protocol_register_driver() - register a driver for a particular network protocol
*/
s32 net_protocol_register_driver(const net_protocol_t proto, const char * const name,
                                 net_rx_fn rx, net_tx_fn tx, net_addr_compare_fn addr_compare,
                                 net_packet_alloc_fn packet_alloc)
{
    net_proto_driver_t *driver = (net_proto_driver_t *) CHECKED_KMALLOC(sizeof(net_proto_driver_t)),
                        *p;

    driver->proto           = proto;
    driver->name            = strdup(name);
    driver->rx              = rx ? rx : net_rx_unimplemented;
    driver->tx              = tx ? tx : net_tx_unimplemented;
    driver->addr_compare    = addr_compare ? addr_compare : net_addr_compare_unimplemented;
    driver->packet_alloc    = packet_alloc ? packet_alloc : net_packet_alloc_unimplemented;

    driver->next = NULL;

    if(!driver->name)
    {
        kfree(driver);
        return EINVAL;
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
    net_rx_unimplemented() - default handler for <proto>_rx()
*/
s32 net_rx_unimplemented(net_packet_t *packet)
{
    UNUSED(packet);
    return ENOSYS;
}


/*
    net_tx_unimplemented() - default handler for <proto>_tx()
*/
s32 net_tx_unimplemented(const net_address_t *src, const net_address_t *dest, net_packet_t *packet)
{
    UNUSED(src);
    UNUSED(dest);
    UNUSED(packet);
    return ENOSYS;
}


/*
    net_packet_alloc_unimplemented() - default handler for <proto>_packet_alloc()
    FIXME - maybe remove this?  I don't think it's used any longer
*/
s32 net_packet_alloc_unimplemented(const net_address_t * const addr, ku32 len,
                                   net_iface_t * const iface, net_packet_t **packet)
{
    UNUSED(addr);
    UNUSED(len);
    UNUSED(iface);
    UNUSED(packet);
    return ENOSYS;
}


/*
    net_addr_compare_unimplemented() - default handler for <proto>_addr_compare()
*/
s32 net_addr_compare_unimplemented(const net_address_t * const a1, const net_address_t * const a2)
{
    UNUSED(a1);
    UNUSED(a2);
    return ENOSYS;      /* Any nonzero retval indicates that the "addresses" do not match */
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


/*
    net_protocol_from_address() - get the protocol associated with a network address.
*/
net_protocol_t net_protocol_from_address(const net_address_t * const addr)
{
    switch(net_address_get_type(addr))
    {
        case na_ethernet:   return np_ethernet;
        case na_ipv4:       return np_ipv4;
        case na_ipv6:       return np_ipv6;

        default:            return np_unknown;
    }
}


/*
    net_protocol_hwproto_from_address() - get the hardware protocol associated with a network
    address.  Involves a routing lookup.
*/
net_protocol_t net_protocol_hwproto_from_address(const net_address_t * const addr)
{
    // FIXME
    UNUSED(addr);

    return np_unknown;
}


/*
    net_protocol_addr_compare() - compare two protocol addresses, using a call to the appropriate
    protocol driver.
*/
s32 net_protocol_addr_compare(const net_protocol_t proto, const net_address_t * const a1,
                              const net_address_t * const a2)
{
    net_proto_driver_t * const drv = net_protocol_get_driver(proto);

    if(!drv || (proto == np_unknown))
        return -1;      /* Mismatch */

    return drv->addr_compare(a1, a2);
}


/*
    net_protocol_rx() - call the appropriate protocol driver to handle a received packet.
*/
s32 net_protocol_rx(net_packet_t * const packet)
{
    net_proto_driver_t *driver = net_protocol_get_driver(net_packet_get_proto(packet));

    if(driver)
        return driver->rx(packet);

    return EPROTONOSUPPORT;
}
