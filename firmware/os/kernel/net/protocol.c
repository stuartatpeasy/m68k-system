/*
    Network protocol abstraction

    Part of ayumos


    (c) Stuart Wallace, May 2016.

    FIXME: remove debug printf() and #include of stdio
    TODO: use a flat array, of size (np_invalid - np_unknown + 1) of protocol drivers.  Pre-init
          all array members to point to a "null" driver.  This will make get_protocol_get_driver
          much faster.
*/

#include <kernel/net/protocol.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/net/packet.h>
#include <kernel/net/route.h>
#include <klibc/stdio.h>
#include <klibc/string.h>
#include <klibc/strings.h>


/* Network protocol driver */
typedef struct net_proto_driver net_proto_driver_t;
struct net_proto_driver
{
    net_protocol_t          proto;
    const char *            name;
    net_proto_fns_t         fn;
    net_proto_driver_t *    next;
};


s32 net_rx_unimplemented(net_address_t *src, net_address_t *dest, net_packet_t *packet);
s32 net_tx_unimplemented(net_address_t *src, net_address_t *dest, net_packet_t *packet);
s32 net_packet_alloc_unimplemented(const net_address_t * const addr, ku32 len,
                                   net_iface_t * const iface, net_packet_t **packet);
s32 net_addr_compare_unimplemented(const net_address_t * const a1, const net_address_t * const a2);
s32 net_route_get_iface_unimplemented(const net_address_t * const addr, net_iface_t **iface);
net_proto_driver_t *net_protocol_get_driver(const net_protocol_t proto);


net_proto_driver_t *g_net_proto_drivers;


/*
    net_protocol_register_driver() - register a driver for a particular network protocol
*/
s32 net_protocol_register_driver(const net_protocol_t proto, const char * const name,
                                 net_proto_fns_t * const f)
{
    net_proto_driver_t *driver = (net_proto_driver_t *) CHECKED_KMALLOC(sizeof(net_proto_driver_t)),
                        *p;

    driver->proto           = proto;
    driver->name            = strdup(name);
    memcpy(&driver->fn, f, sizeof(net_proto_fns_t));

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
    net_proto_fns_struct_init() - initialise a net_proto_fns_t struct by setting all members to
    point to the _unimplemented() versions of the respective functions.
*/
void net_proto_fns_struct_init(net_proto_fns_t * const f)
{
    f->rx               = net_rx_unimplemented;
    f->tx               = net_tx_unimplemented;
    f->addr_compare     = net_addr_compare_unimplemented;
    f->packet_alloc     = net_packet_alloc_unimplemented;
    f->route_get_iface  = net_route_get_iface_unimplemented;
}


/*
    net_protocol_rx() - call the appropriate protocol driver to handle a received packet.
*/
s32 net_protocol_rx(net_address_t *src, net_address_t *dest, net_packet_t * const packet)
{
    net_proto_driver_t *driver = net_protocol_get_driver(net_packet_get_proto(packet));

    if(driver)
        return driver->fn.rx(src, dest, packet);

    return EPROTONOSUPPORT;
}


/*
    net_protocol_tx() - obtain an appropriate protocol driver for a packet, and pass the packet to
    the driver's tx() function.
*/
s32 net_protocol_tx(net_address_t *src, net_address_t *dest, net_packet_t *packet)
{
    const net_proto_driver_t *driver = net_protocol_get_driver(net_address_get_proto(dest));

    if(!driver)
        return EPROTONOSUPPORT;

    return driver->fn.tx(src, dest, packet);
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

    return drv->fn.addr_compare(a1, a2);
}


/*
    net_protocol_packet_alloc() - allocate a packet object and allocate a buffer of the specified
    length for the payload.
*/
s32 net_protocol_packet_alloc(const net_protocol_t proto, const net_address_t * const addr,
                              ku32 len, net_iface_t * const iface, net_packet_t **packet)
{
    net_proto_driver_t * const drv = net_protocol_get_driver(proto);

    if(!drv || (proto == np_unknown))
        return EPROTONOSUPPORT;

    return drv->fn.packet_alloc(addr, len, iface, packet);
}


/*
    net_route_get_iface() - get the interface associated with a particular (protocol) address.
*/
s32 net_route_get_iface(const net_address_t * const addr, net_iface_t **iface)
{
    net_proto_driver_t * const drv = net_protocol_get_driver(net_address_get_proto(addr));

    if(!drv)
        return EPROTONOSUPPORT;

    return drv->fn.route_get_iface(addr, iface);
}


/*
    net_rx_unimplemented() - default handler for <proto>_rx()
*/
s32 net_rx_unimplemented(net_address_t *src, net_address_t *dest, net_packet_t *packet)
{
    UNUSED(src);
    UNUSED(dest);
    UNUSED(packet);
    return ENOSYS;
}


/*
    net_tx_unimplemented() - default handler for <proto>_tx()
*/
s32 net_tx_unimplemented(net_address_t *src, net_address_t *dest, net_packet_t *packet)
{
    UNUSED(src);
    UNUSED(dest);
    UNUSED(packet);
    return ENOSYS;
}


/*
    net_packet_alloc_unimplemented() - default handler for <proto>_packet_alloc()
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
    net_protocol_route_get_iface_unimplemented() - default handler for <proto>_route_get_iface()
*/
s32 net_route_get_iface_unimplemented(const net_address_t * const addr, net_iface_t **iface)
{
    UNUSED(addr);
    UNUSED(iface);
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
    net_iface_t *iface;

    return (net_route_get_iface(addr, &iface) == SUCCESS) ?
                net_interface_get_proto(iface) : np_unknown;
}
