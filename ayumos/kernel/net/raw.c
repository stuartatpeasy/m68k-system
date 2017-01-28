/*
    "Raw" protocol implementation

    Part of ayumos


    (c) Stuart Wallace, May 2016.
*/

#ifdef WITH_NETWORKING

#include <kernel/include/memory/kmalloc.h>
#include <kernel/include/net/raw.h>
#include <kernel/include/net/packet.h>
#include <kernel/include/net/protocol.h>


/*
    raw_init() - initialise the raw "protocol" driver.  This protocol deals with raw packets, i.e.
    ignores all protocol-specific data.
*/
s32 raw_init()
{
    net_proto_fns_t fns;

    net_proto_fns_struct_init(&fns);

    fns.packet_alloc = raw_packet_alloc;

    return net_protocol_register_driver(np_raw, "raw", &fns);
}


/*
    raw_packet_alloc() - allocate a "raw" packet, i.e. just a fixed-length buffer without a specific
    associated protocol.  "Raw" is a top-level protocol driver (i.e. a raw packet cannot be
    encapsulated any further), so we actually allocate memory here.  Packets allocated for other
    protocols will eventually end up being allocated by this function after they have been fully
    encapsulated, e.g.
        tcp_packet_alloc() -> ipv4_packet_alloc() -> eth_packet_alloc() -> raw_packet_alloc()
*/
s32 raw_packet_alloc(const net_address_t * const addr, ku32 len, net_iface_t * const iface,
                     net_packet_t **packet)
{
    ks32 ret = net_packet_alloc(addr, len, iface, packet);

    if(ret != SUCCESS)
        return ret;

    net_packet_set_proto(*packet, np_raw);

    return SUCCESS;
}

#endif /* WITH_NETWORKING */
