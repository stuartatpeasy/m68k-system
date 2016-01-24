/*
    TCP implementation

    Part of ayumos


    (c) Stuart Wallace, January 2016.
*/

#include <kernel/net/tcp.h>


/*
    tcp_handle_packet() - handle an incoming TCP packet.
*/
s32 tcp_handle_packet(net_iface_t *iface, net_packet_t *packet, net_packet_t **response)
{
    UNUSED(iface);
    UNUSED(packet);
    UNUSED(response);

    return SUCCESS;
}
