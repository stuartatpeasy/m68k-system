/*
    UDP implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/udp.h>
#include <kernel/net/dhcp.h>
#include <kernel/net/packet.h>


/*
    udp_rx() - handle an incoming UDP packet.
*/
s32 udp_rx(net_packet_t *packet)
{
    udp_hdr_t *hdr = (udp_hdr_t *) net_packet_get_start(packet);

    net_packet_consume(sizeof(udp_hdr_t), packet);

    if(hdr->dest_port == DHCP_CLIENT_PORT)
        return dhcp_rx(packet);

    /* FIXME - do something else with the packet maybe? */

    return SUCCESS;
}


/*
    udp_tx() - transmit a UDP packet.
*/
s32 udp_tx(const net_address_t *src, const net_address_t *dest, net_packet_t *packet)
{
    udp_hdr_t *hdr;
    ipv4_address_t *src_addr, *dest_addr;

    if((src->type != na_ipv4) || (dest->type != na_ipv4))
        return EPROTONOSUPPORT;

    net_packet_insert(sizeof(udp_hdr_t), packet);

    hdr = (udp_hdr_t *) net_packet_get_start(packet);

    src_addr    = (ipv4_address_t *) net_address_get_address(src);
    dest_addr   = (ipv4_address_t *) net_address_get_address(dest);

    hdr->src_port   = N2BE16(src_addr->port);
    hdr->dest_port  = N2BE16(dest_addr->port);
    hdr->len        = N2BE16(net_packet_get_len(packet));
    hdr->cksum      = 0;

    return net_tx(src, dest, packet);
}


/*
    udp_packet_alloc() - allocate a packet for transmission, to contain a payload of the
    specified length.
*/
s32 udp_packet_alloc(const net_address_t * const addr, ku32 len, net_iface_t *iface,
                     net_packet_t **packet)
{
    ks32 ret = net_packet_alloc(net_address_get_proto(addr), addr, sizeof(udp_hdr_t) + len, iface,
                                packet);
    if(ret != SUCCESS)
        return ret;

    net_packet_consume(sizeof(udp_hdr_t), *packet);
    net_packet_set_proto(np_udp, *packet);

    return SUCCESS;
}
