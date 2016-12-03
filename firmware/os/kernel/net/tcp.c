/*
    TCP implementation

    Part of ayumos


    (c) Stuart Wallace, January 2016.
*/

#include <kernel/include/net/tcp.h>
#include <kernel/include/net/protocol.h>


/*
    tcp_init() - initialise the TCP protocol driver.
*/
s32 tcp_init()
{
    net_proto_fns_t fns;

    net_proto_fns_struct_init(&fns);

    fns.rx = tcp_rx;

    return net_protocol_register_driver(np_tcp, "TCP", &fns);
}


/*
    tcp_handle_packet() - handle an incoming TCP packet.
*/
s32 tcp_rx(net_address_t *src, net_address_t *dest, net_packet_t *packet)
{
    UNUSED(src);
    UNUSED(dest);
    UNUSED(packet);

    return SUCCESS;
}
