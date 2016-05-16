/*
    Network packet abstraction

    Part of ayumos


    (c) Stuart Wallace, May 2016.

    FIXME - net_packet_alloc() should get proto driver and call packet_alloc() on it, then remove #includes of various protocols
*/

#include <kernel/net/packet.h>
#include <kernel/net/ethernet.h>
#include <kernel/net/ipv4.h>
#include <kernel/net/udp.h>
#include <kernel/memory/kmalloc.h>


struct net_packet
{
    net_iface_t *           iface;
    net_protocol_t          proto;
    void *                  start;
    u32                     len;        /* Actual amount of data in the buffer  */
    buffer_t                raw;
};


/*
    net_packet_alloc() - allocate a packet object and allocate a buffer of the specified length for
    the payload.
*/
s32 net_packet_alloc(const net_protocol_t proto, const net_address_t * const addr, ku32 len,
                     net_iface_t * const iface, net_packet_t **packet)
{
    /* MASSIVE FIXME - I have no idea how to complete this function at the moment */
    switch(proto)
    {
        case np_udp:        return udp_packet_alloc(addr, len, iface, packet);
        case np_ipv4:       return ipv4_packet_alloc(addr, len, iface, packet);
        case np_ethernet:   return eth_packet_alloc(addr, len, iface, packet);
        case np_raw:        return net_packet_alloc_raw(addr, len, iface, packet);

        default:        return EPROTONOSUPPORT;
    }
}


/*
    net_packet_alloc_raw() - allocate a "raw" packet, i.e. just a fixed-length buffer without a
    specific associated protocol.
*/
s32 net_packet_alloc_raw(const net_address_t * const addr, ku32 len, net_iface_t * const iface,
                         net_packet_t **packet)
{
    s32 ret;
    net_packet_t *p = CHECKED_KMALLOC(sizeof(net_packet_t));

    ret = buffer_init(len, &p->raw);
    if(ret != SUCCESS)
    {
        kfree(p);
        return ret;
    }

    /* If an interface was supplied, use it; otherwise look up based on address. */
    p->iface = iface ? iface : net_route_get(addr);
    if(!p->iface)
        return EHOSTUNREACH;

    p->proto = np_raw;
    p->start = p->raw.data;
    p->len = 0;

    *packet = p;

    return SUCCESS;

}


/*
    net_packet_free() - destroy an object created by net_packet_alloc().
*/
void net_packet_free(net_packet_t *packet)
{
    buffer_deinit(&packet->raw);
    kfree(packet);
}


/*
    net_packet_reset() - "reset" a packet by setting its "start" pointer to the beginning of its
    buffer, and setting its "length" member to the size of the buffer.  This enables a packet
    object to be re-used.
*/
void net_packet_reset(net_packet_t *packet)
{
    packet->start = packet->raw.data;
    packet->len = packet->raw.len;
}


/*
    net_packet_get_interface() - return the interface associated with a packet.
*/
net_iface_t *net_packet_get_interface(net_packet_t * const packet)
{
    return packet->iface;
}


void net_packet_set_interface(net_packet_t * const packet, net_iface_t * const iface)
{
    packet->iface = iface;
}


/*
    net_packet_get_start() - get a pointer to the current start of the payload in a packet.
*/
void *net_packet_get_start(net_packet_t * const packet)
{
    return packet->start;
}


/*
    net_packet_get_len() - get the length of the payload in a packet object.
*/
u32 net_packet_get_len(const net_packet_t * const packet)
{
    return packet->len;
}


/*
    net_packet_get_buffer_len() - get the length of the storage buffer in a packet object.
*/
u32 net_packet_get_buffer_len(const net_packet_t * const packet)
{
    return packet->raw.len;
}


/*
    net_packet_set_len() - set the payload length of a packet.  This cannot exceed the size of the
    buffer associated with the packet.
*/
s32 net_packet_set_len(net_packet_t * const packet, ku32 new_len)
{
    if(new_len > packet->raw.len)
        return EINVAL;

    packet->len = new_len;
    return SUCCESS;
}


/*
    net_packet_get_proto() - get the protocol associated with a packet
*/
net_protocol_t net_packet_get_proto(net_packet_t * const packet)
{
    return packet->proto;
}


/*
    net_packet_set_proto() - set the protocol associated with a packet.
*/
void net_packet_set_proto(const net_protocol_t proto, net_packet_t * const packet)
{
    packet->proto = proto;
}

/*
    net_packet_encapsulate() - set a packet's protocol and "insert" bytes at the start of its buffer
    by calling net_packet_insert().
*/
s32 net_packet_encapsulate(const net_protocol_t proto, ku32 len, net_packet_t * const packet)
{
    packet->proto = proto;
    return net_packet_insert(len, packet);
}


/*
    net_packet_insert() - "insert" bytes at the start of a packet by adjusting its start and len
    members.  This is useful when a packet passes up the networking stack, and needs to be
    encapsulated in headers for other protocols.
*/
s32 net_packet_insert(ku32 len, net_packet_t * const packet)
{
    /* FIXME - check that we're not underflowing */
    packet->start -= len;
    packet->len += len;

    return SUCCESS;
}


/*
    net_packet_consume() - "consume" bytes at the start of a packet by adjusting its start and len
    members.  This is useful when a packet passes down the networking stack, and needs to be
    decapsulated.
*/
s32 net_packet_consume(ku32 len, net_packet_t * const packet)
{
    /* FIXME - check that we're not overflowing */
    packet->start += len;
    packet->len -= len;

    return SUCCESS;
}
