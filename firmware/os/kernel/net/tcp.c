/*
    TCP implementation

    Part of ayumos


    (c) Stuart Wallace, January 2016.
*/

#include <kernel/net/tcp.h>


/*
    tcp_handle_packet() - handle an incoming TCP packet.
*/
s32 tcp_rx(net_packet_t *packet)
{
    UNUSED(packet);

    return SUCCESS;
}
