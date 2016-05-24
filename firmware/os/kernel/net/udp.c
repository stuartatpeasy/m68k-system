/*
    UDP implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/udp.h>
#include <kernel/net/dhcp.h>
#include <kernel/net/packet.h>


/*
    udp_init() - initialise the UDP protocol driver.
*/
s32 udp_init()
{
    net_proto_fns_t fns;

    net_proto_fns_struct_init(&fns);

    fns.rx              = udp_rx;
    fns.tx              = udp_tx;
    fns.packet_alloc    = udp_packet_alloc;

    return net_protocol_register_driver(np_udp, "UDP", &fns);
}


/*
    udp_rx() - handle an incoming UDP packet.
*/
s32 udp_rx(net_address_t *src, net_address_t *dest, net_packet_t *packet)
{
    s32 ret;
    udp_hdr_t *hdr = (udp_hdr_t *) net_packet_get_start(packet);

    ret = net_packet_consume(packet, sizeof(udp_hdr_t));
    if(ret != SUCCESS)
        return ret;

    /* FIXME - this handling of port numbers is ugly and broken; should incorporate portnum in net_address_t? */
    ipv4_addr_set_port((ipv4_address_t *) net_address_get_address(src), hdr->src_port);
    ipv4_addr_set_port((ipv4_address_t *) net_address_get_address(dest), hdr->dest_port);

    if(hdr->dest_port == DHCP_CLIENT_PORT)
        return dhcp_rx(packet);

    /* FIXME - do something else with the packet maybe? */

    return SUCCESS;
}


/*
    udp_tx() - transmit a UDP packet.
*/
s32 udp_tx(net_address_t *src, net_address_t *dest, net_packet_t *packet)
{
    udp_hdr_t *hdr;
    s32 ret;

    /* FIXME - make this work with "IP-family" addresses, not just IPv4 addresses specifically */
    if((net_address_get_type(src) != na_ipv4) || (net_address_get_type(dest) != na_ipv4))
        return EAFNOSUPPORT;

    ret = net_packet_insert(packet, sizeof(udp_hdr_t));
    if(ret != SUCCESS)
        return ret;

    hdr = (udp_hdr_t *) net_packet_get_start(packet);

    hdr->src_port   = N2BE16(ipv4_get_port(src));
    hdr->dest_port  = N2BE16(ipv4_get_port(dest));
    hdr->len        = N2BE16(net_packet_get_len(packet));
    hdr->cksum      = 0;

    return net_protocol_tx(src, dest, packet);
}


/*
    udp_packet_alloc() - allocate a packet for transmission, to contain a payload of the
    specified length.
*/
s32 udp_packet_alloc(const net_address_t * const addr, ku32 len, net_iface_t *iface,
                     net_packet_t **packet)
{
    ks32 ret = net_protocol_packet_alloc(net_address_get_proto(addr), addr, sizeof(udp_hdr_t) + len,
                                         iface, packet);
    if(ret != SUCCESS)
        return ret;

    net_packet_set_proto(*packet, np_udp);

    return net_packet_consume(*packet, sizeof(udp_hdr_t));
}
