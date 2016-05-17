/*
    Network packet abstraction

    Part of ayumos


    (c) Stuart Wallace, May 2016.
*/

#include <kernel/net/packet.h>
#include <kernel/net/route.h>
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


s32 net_packet_create(ku32 len, net_packet_t **packet);


/*
    net_packet_create() - create a packet object and allocate its buffer.  Private to this module;
    used by net_packet_alloc() and net_packet_clone().
*/
s32 net_packet_create(ku32 len, net_packet_t **packet)
{
    s32 ret;
    net_packet_t *p = CHECKED_KMALLOC(sizeof(net_packet_t));

    ret = buffer_init(len, &p->raw);
    if(ret != SUCCESS)
    {
        kfree(p);
        return ret;
    }

    *packet = p;
    return SUCCESS;
}


/*
    net_packet_alloc() - allocate a packet object and allocate a buffer of the specified length for
    the payload.
*/
s32 net_packet_alloc(const net_address_t * const addr, ku32 len, net_iface_t * const iface,
                     net_packet_t **packet)
{
    net_packet_t *p;
    ks32 ret = net_packet_create(len, &p);

    if(ret != SUCCESS)
        return ret;

    /* If an interface was supplied, use it; otherwise look up based on address. */
    p->iface = iface ? iface : net_route_get(addr);
    if(!p->iface)
        return EHOSTUNREACH;

    net_packet_reset(p);
    net_packet_set_proto(p, np_unknown);

    *packet = p;

    return SUCCESS;
}


/*
    net_packet_clone() - clone net_packet_t object "packet" into "new_packet".
*/
s32 net_packet_clone(const net_packet_t * const packet, net_packet_t ** new_packet)
{
    net_packet_t *newp;
    ks32 ret = net_packet_create(packet->raw.len, &newp);

    if(ret != SUCCESS)
        return ret;

    newp->iface = packet->iface;
    newp->proto = packet->proto;
    newp->start = packet->start;
    newp->len = packet->len;

    *new_packet = newp;

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
    packet->len = 0;
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
void net_packet_set_proto(net_packet_t * const packet, const net_protocol_t proto)
{
    packet->proto = proto;
}

/*
    net_packet_encapsulate() - set a packet's protocol and "insert" bytes at the start of its buffer
    by calling net_packet_insert().
*/
s32 net_packet_encapsulate(net_packet_t * const packet, const net_protocol_t proto, ku32 len)
{
    packet->proto = proto;
    return net_packet_insert(packet, len);
}


/*
    net_packet_insert() - "insert" bytes at the start of a packet by adjusting its start and len
    members.  This is useful when a packet passes up the networking stack, and needs to be
    encapsulated in headers for other protocols.
*/
s32 net_packet_insert(net_packet_t * const packet, ku32 len)
{
    if((packet->len) + len > packet->raw.len)
        return EINVAL;      /* Underflow */

    packet->start -= len;
    packet->len += len;

    return SUCCESS;
}


/*
    net_packet_consume() - "consume" bytes at the start of a packet by adjusting its start and len
    members.  This is useful when a packet passes down the networking stack, and needs to be
    decapsulated.
*/
s32 net_packet_consume(net_packet_t * const packet, ku32 len)
{
    if(packet->len < len)
        return EINVAL;      /* Overflow */

    packet->start += len;
    packet->len -= len;

    return SUCCESS;
}
