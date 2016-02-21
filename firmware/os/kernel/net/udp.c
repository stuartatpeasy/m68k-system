/*
    UDP implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/udp.h>


/*
    udp_rx() - handle an incoming UDP packet.
*/
s32 udp_rx(net_packet_t *packet)
{
    UNUSED(packet);

    return SUCCESS;
}


/*
    udp_tx() - transmit a UDP packet.
*/
s32 udp_tx(const net_address_t *src, const net_address_t *dest, net_packet_t *packet)
{
    udp_hdr_t *hdr;
    ipv4_address_t *src_addr, *dest_addr;

    puts("udp_tx()");

    if((src->type != na_ipv4) || (dest->type != na_ipv4))
        return EPROTONOSUPPORT;

    packet->start -= sizeof(udp_hdr_t);
    packet->len += sizeof(udp_hdr_t);

    hdr = (udp_hdr_t *) packet->start;

    src_addr    = (ipv4_address_t *) &src->addr;
    dest_addr   = (ipv4_address_t *) &dest->addr;

    hdr->src_port   = N2BE16(src_addr->port);
    hdr->dest_port  = N2BE16(dest_addr->port);
    hdr->len        = N2BE16(sizeof(udp_hdr_t) + packet->len);
    hdr->cksum      = 0;

    return ipv4_tx(src, dest, packet);
}


/*
    udp_alloc_packet() - allocate a packet for transmission, to contain a payload of the
    specified length.
*/
s32 udp_alloc_packet(net_iface_t *iface, ku32 len, net_packet_t **packet)
{
    ks32 ret = ipv4_alloc_packet(iface, sizeof(udp_hdr_t) + len, packet);
    if(ret != SUCCESS)
        return ret;

    (*packet)->start += sizeof(udp_hdr_t);
    (*packet)->len -= sizeof(udp_hdr_t);
    (*packet)->proto = np_udp;

    return SUCCESS;
}
