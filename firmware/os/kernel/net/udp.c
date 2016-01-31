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
