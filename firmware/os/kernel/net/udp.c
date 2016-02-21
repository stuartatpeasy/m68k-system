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
    udp_send() - send a UDP packet
*/
s32 udp_send(net_iface_t *iface, const net_address_t *src_addr, const ipv4_port_t src_port,
             const net_address_t *dest_addr, const ipv4_port_t dest_port, buffer_t *payload)
{
    UNUSED(iface);
    UNUSED(src_addr);
    UNUSED(src_port);
    UNUSED(dest_addr);
    UNUSED(dest_port);
    UNUSED(payload);

    return SUCCESS;
}


/*
    udp_alloc_packet() - allocate a packet for transmission, to contain a payload of the
    specified length.
*/
s32 udp_alloc_packet(net_iface_t *iface, ku32 len, net_packet_t **packet)
{
    ks32 ret = iface->driver->alloc_packet(iface, sizeof(udp_hdr_t) + len, packet);
    if(ret != SUCCESS)
        return ret;

    (*packet)->start += sizeof(udp_hdr_t);
    (*packet)->len += sizeof(udp_hdr_t);

    return SUCCESS;
}
