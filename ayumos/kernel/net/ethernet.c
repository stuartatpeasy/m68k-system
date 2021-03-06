/*
    Ethernet implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#ifdef WITH_NETWORKING
#ifdef WITH_NET_ETHERNET

#include <kernel/include/net/arp.h>
#include <kernel/include/net/ethernet.h>
#include <kernel/include/net/interface.h>
#include <kernel/include/net/ipv4.h>
#include <kernel/include/net/packet.h>
#include <klibc/include/stdio.h>
#include <klibc/include/string.h>
#include <klibc/include/strings.h>


s32 eth_rx(net_address_t *src, net_address_t *dest, net_packet_t *packet);
s32 eth_tx(net_address_t *src, net_address_t *dest, net_packet_t *packet);
s32 eth_packet_alloc(const net_address_t * const addr, ku32 len, net_iface_t *iface,
                     net_packet_t **packet);


/* MAC address representing the broadcast address */
const mac_addr_t eth_mac_broadcast =
{
    .b = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
};


/*
    eth_init() - initialise Ethernet protocol driver
*/
s32 eth_init()
{
    net_proto_fns_t fns;

    net_proto_fns_struct_init(&fns);

    fns.rx              = eth_rx;
    fns.tx              = eth_tx;
    fns.addr_compare    = eth_addr_compare;
    fns.packet_alloc    = eth_packet_alloc;

    return net_protocol_register_driver(np_ethernet, "Ethernet", &fns);
}


/*
    eth_rx() - handle an incoming Ethernet packet.  Ethernet is a layer 2 protocol, meaning that
    this driver may be the first one called after a packet is received by a hardware driver.
    In that case, packet->proto will be np_raw, and the src and dest addresses will be na_unknown;
    we will therefore set src and dest to the source and destination Ethernet addresses contained in
    the packet.
*/
s32 eth_rx(net_address_t *src, net_address_t *dest, net_packet_t *packet)
{
    s32 ret;
    const eth_hdr_t * const ehdr = (eth_hdr_t *) net_packet_get_start(packet);

    ret = net_packet_consume(packet, sizeof(eth_hdr_t));
    if(ret != SUCCESS)
        return ret;

    if(net_address_get_proto(src) == np_unknown)
        eth_make_addr(&ehdr->src, src);

    if(net_address_get_proto(dest) == np_unknown)
        eth_make_addr(&ehdr->dest, dest);

    net_packet_set_proto(packet, eth_proto_from_ethertype(ehdr->type));

    return net_protocol_rx(src, dest, packet);
}


/*
    eth_tx() - add an Ethernet header to the supplied frame and transmit it on the specified
    interface.
*/
s32 eth_tx(net_address_t *src, net_address_t *dest, net_packet_t *packet)
{
    eth_hdr_t *hdr;
    net_protocol_t proto;
    s32 ret;

    /* Handle NULL source addresses: use the default address for the specified interface */
    if(!src)
    {
        /* No source address specified.  Use the default hardware address for the interface */
        const net_iface_t * const iface = net_packet_get_interface(packet);

        if(!iface)
            return -EHOSTUNREACH;   /* No source address and no interface - packet unrouteable. */

        src = (net_address_t *) net_interface_get_hw_addr(iface);
    }

    /* Source and destination addresses must be an Ethernet address */
    if((net_address_get_type(src) != na_ethernet) || (net_address_get_type(dest) != na_ethernet))
        return -EAFNOSUPPORT;

    proto = net_packet_get_proto(packet);

    ret = net_packet_encapsulate(packet, np_ethernet, sizeof(eth_hdr_t));
    if(ret != SUCCESS)
        return ret;

    hdr = (eth_hdr_t *) net_packet_get_start(packet);
    hdr->type = eth_ethertype_from_proto(proto);

    if(hdr->type == ethertype_unknown)
        return -EPROTONOSUPPORT;

    hdr->src = *eth_get_addr(src);
    hdr->dest = *eth_get_addr(dest);

    return net_tx(packet);
}


/*
    eth_make_addr() - populate a net_address_t object with a MAC address and return it.
*/
net_address_t *eth_make_addr(const mac_addr_t * const mac, net_address_t *addr)
{
    void * const addr_buf = (void *) net_address_get_address(addr);

    net_address_set_type(na_ethernet, addr);
    bzero(addr_buf, sizeof(net_addr_t));

    if(mac != NULL)
        memcpy(addr_buf, mac, sizeof(mac_addr_t));

    return addr;
}


/*
    eth_make_broadcast_address(): populate a net_address_t object with the Ethernet broadcast
    address (ff:ff:ff:ff:ff:ff) and return it.
*/
net_address_t *eth_make_broadcast_addr(net_address_t *addr)
{
    eth_make_addr(&eth_mac_broadcast, addr);

    return addr;
}


/*
    eth_get_addr() - if the supplied net_address_t object represents an Ethernet address, return a
    ptr to the MAC address part of the object; otherwise, return NULL.
*/
const mac_addr_t *eth_get_addr(const net_address_t * const addr)
{
    if(net_address_get_type(addr) != na_ethernet)
        return NULL;

    return (mac_addr_t *) net_address_get_address(addr);
}


/*
    eth_packet_alloc() - allocate a net_packet_t object large enough to contain an Ethernet header
    and a payload of the specified length.
*/
s32 eth_packet_alloc(const net_address_t * const addr, ku32 len, net_iface_t *iface,
                     net_packet_t **packet)
{
    ks32 ret = net_protocol_packet_alloc(np_raw, addr, sizeof(eth_hdr_t) + len, iface, packet);

    if(ret != SUCCESS)
        return ret;

    net_packet_set_proto(*packet, np_ethernet);

    return net_packet_consume(*packet, sizeof(eth_hdr_t));
}


/*
    eth_addr_compare() - compare two Ethernet addresses.
*/
s32 eth_addr_compare(const net_address_t * const a1, const net_address_t * const a2)
{
    if((net_address_get_type(a1) != na_ethernet) || (net_address_get_type(a2) != na_ethernet))
        return -1;      /* Mismatch */

    return memcmp(net_address_get_address(a1), net_address_get_address(a2), sizeof(mac_addr_t));
}


/*
    eth_proto_from_ethertype() - given an ethertype value, return the corresponding np_* protocol
    constant.
*/
net_protocol_t eth_proto_from_ethertype(ku16 ethertype)
{
    switch(ethertype)
    {
        case ethertype_ipv4:
            return np_ipv4;

        case ethertype_arp:
            return np_arp;

        default:
            return np_unknown;
    }
}


/*
    eth_ethertype_from_proto() - given a net_protocol_t value, return the corresponding ethertype
    value.
*/
ethertype_t eth_ethertype_from_proto(const net_protocol_t proto)
{
    switch(proto)
    {
        case np_ipv4:
            return ethertype_ipv4;

        case np_arp:
            return ethertype_arp;

        default:
            return ethertype_unknown;
    }
}


/*
    eth_print_addr() - write addr to buf
*/
s32 eth_print_addr(const net_address_t *addr, char *buf, s32 len)
{
    const mac_addr_t * const m = (const mac_addr_t *) net_address_get_address(addr);

    return snprintf(buf, len, "%02x:%02x:%02x:%02x:%02x:%02x",
                    m->b[0], m->b[1], m->b[2], m->b[3], m->b[4], m->b[5]);
}

#endif /* WITH_NET_ETHERNET */
#endif /* WITH_NETWORKING */
