/*
    Ethernet implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/ethernet.h>
#include <kernel/net/ipv4.h>
#include <kernel/net/arp.h>
#include <kernel/net/packet.h>
#include <klibc/stdio.h>
#include <klibc/strings.h>


const net_address_t g_eth_broadcast =
{
    .type = na_ethernet,
    .addr.addr_bytes[0] = 0xff,
    .addr.addr_bytes[1] = 0xff,
    .addr.addr_bytes[2] = 0xff,
    .addr.addr_bytes[3] = 0xff,
    .addr.addr_bytes[4] = 0xff,
    .addr.addr_bytes[5] = 0xff
};


/*
    eth_init() - initialise Ethernet protocol driver
*/
s32 eth_init()
{
    return net_protocol_register_driver(np_ethernet, "Ethernet", eth_rx, eth_tx, eth_addr_compare);
}


/*
    eth_rx() - handle an incoming Ethernet packet.
*/
s32 eth_rx(net_iface_t *iface, net_packet_t *packet)
{
    const eth_hdr_t * const ehdr = (eth_hdr_t *) net_packet_get_start(packet);

    net_packet_consume(sizeof(eth_hdr_t), packet);

    // FIXME - do eth_proto_from_ethertype() then a driver lookup here
    switch(ehdr->type)
    {
        case ethertype_ipv4:
            net_packet_set_proto(np_ipv4, packet);
            return ipv4_rx(iface, packet);
            break;

        case ethertype_arp:
            net_packet_set_proto(np_arp, packet);
            return arp_rx(iface, packet);
            break;
    }

    return EPROTONOSUPPORT;
}


/*
    eth_tx() - add an Ethernet header to the supplied frame and transmit it on the specified
    interface.
*/
s32 eth_tx(const net_address_t *src, const net_address_t *dest, net_packet_t *packet)
{
    eth_hdr_t *hdr;
    net_protocol_t proto;

    /* Source and dest addresses must be na_ethernet */
    if((net_address_get_type(src) != na_ethernet) || (net_address_get_type(dest) != na_ethernet))
        return EAFNOSUPPORT;

    proto = net_packet_get_proto(packet);

    net_packet_encapsulate(np_ethernet, sizeof(eth_hdr_t), packet);
    hdr = (eth_hdr_t *) net_packet_get_start(packet);

    switch(proto)
    {
        case np_ipv4:
            hdr->type = ethertype_ipv4;
            break;

        case np_arp:
            hdr->type = ethertype_arp;
            break;

        default:
            return EPROTONOSUPPORT;
    }

    /*
        If src is NULL, send from the default address of the interface; otherwise send from the
        specified address.
    */
    hdr->src = *eth_get_addr(src);
    hdr->dest = *eth_get_addr(dest);

    return net_tx(src, dest, packet);
}

#if 0   // FIXME
/*
    eth_reply() - assume that *packet contains a received packet which has been modified in some
    way; swap its source and destination addresses and transmit it.
*/
s32 eth_reply(net_packet_t *packet)
{
    eth_hdr_t *ehdr;
    net_address_t src, dest;

    packet->start -= sizeof(eth_hdr_t);
    packet->len += sizeof(eth_hdr_t);

    ehdr = (eth_hdr_t *) packet->start;

    eth_make_addr(&ehdr->dest, &src);
    eth_make_addr(NULL, &dest);

    ehdr->dest = *((mac_addr_t *) &dest.addr);
    ehdr->src = *((mac_addr_t *) &src.addr);

    return net_tx(&src, &dest, packet);
}
#endif


/*
    eth_make_addr() - populate a net_address_t object with a MAC address.
*/
void eth_make_addr(mac_addr_t *mac, net_address_t *addr)
{
    net_address_set_type(na_ethernet, addr);
    bzero(&addr->addr, sizeof(net_addr_t));

    if(mac != NULL)
        memcpy(&addr->addr, mac, sizeof(mac_addr_t));
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
    ks32 ret = net_packet_alloc(np_ethernet, addr, sizeof(eth_hdr_t) + len, iface, packet);

    if(ret != SUCCESS)
        return ret;

    net_packet_consume(sizeof(eth_hdr_t), *packet);

    return SUCCESS;
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
    eth_print_addr() - write addr to buf
*/
s32 eth_print_addr(const net_address_t *addr, char *buf, s32 len)
{
    const mac_addr_t * const m = (const mac_addr_t *) net_address_get_address(addr);

    return snprintf(buf, len, "%02x:%02x:%02x:%02x:%02x:%02x",
                    m->b[0], m->b[1], m->b[2], m->b[3], m->b[4], m->b[5]);
}
