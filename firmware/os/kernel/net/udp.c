/*
    UDP implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/udp.h>

/*
    udp_handle_packet() - handle an incoming UDP packet.
*/
s32 udp_handle_packet(net_iface_t *iface, const void *packet, u32 len)
{
    UNUSED(iface);
    UNUSED(packet);
    UNUSED(len);

    return SUCCESS;
}
