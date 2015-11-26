/*
    ICMP implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/icmp.h>


/*
    icmp_handle_packet() - handle an incoming IPv4 packet.
*/
s32 icmp_handle_packet(net_iface_t *iface, const void *packet, u32 len)
{
    UNUSED(iface);
    UNUSED(packet);
    UNUSED(len);

    return SUCCESS;
}
