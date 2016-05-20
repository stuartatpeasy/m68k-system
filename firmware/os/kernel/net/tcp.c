/*
    TCP implementation

    Part of ayumos


    (c) Stuart Wallace, January 2016.
*/

#include <kernel/net/tcp.h>
#include <kernel/net/protocol.h>


/*
    tcp_init() - initialise the TCP protocol driver.
*/
s32 tcp_init()
{
    return net_protocol_register_driver(np_tcp, "TCP", tcp_rx, NULL, NULL, NULL);
}


/*
    tcp_handle_packet() - handle an incoming TCP packet.
*/
s32 tcp_rx(net_packet_t *packet)
{
    UNUSED(packet);

    return SUCCESS;
}
