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
    puts("udp_rx");

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
