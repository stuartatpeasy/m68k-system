/*
    TCP implementation

    Part of ayumos


    (c) Stuart Wallace, January 2016.
*/

#include <kernel/net/tcp.h>


/*
    tcp_handle_packet() - handle an incoming TCP packet.
*/
s32 tcp_handle_packet(net_iface_t *iface, const void *packet, u32 len)
{
    UNUSED(iface);
    UNUSED(packet);
    UNUSED(len);

    return SUCCESS;
}
