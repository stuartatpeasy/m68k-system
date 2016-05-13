/*
    Network interface abstraction

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/device/device.h>
#include <kernel/error.h>
#include <kernel/net/arp.h>
#include <kernel/net/ethernet.h>
#include <kernel/net/interface.h>
#include <kernel/net/ipv4.h>
#include <kernel/net/net.h>
#include <kernel/process.h>
#include <kernel/util/kutil.h>
#include <klibc/strings.h>
#include <klibc/stdio.h>


s32 net_rx_unimplemented(net_packet_t *packet);
s32 net_tx_unimplemented(const net_address_t *src, const net_address_t *dest, net_packet_t *packet);
s32 net_packet_alloc_unimplemented(net_iface_t *iface, ku32 len, net_packet_t **packet);


/*
    net_init() - initialise networking layer
*/
s32 net_init()
{
    s32 ret;


    /* Register protocols */
    ret = net_protocol_init();
    if(ret != SUCCESS)
        return ret;

    return net_interface_init();
}



s32 net_rx_unimplemented(net_packet_t *packet)
{
    UNUSED(packet);
    return ENOSYS;
}


s32 net_tx_unimplemented(const net_address_t *src, const net_address_t *dest, net_packet_t *packet)
{
    UNUSED(src);
    UNUSED(dest);
    UNUSED(packet);
    return ENOSYS;
}


s32 net_packet_alloc_unimplemented(net_iface_t *iface, ku32 len, net_packet_t **packet)
{
    UNUSED(iface);
    UNUSED(len);
    UNUSED(packet);
    return ENOSYS;
}


/*
    net_tx() - send a packet over an interface.
*/
s32 net_tx(const net_address_t *src, const net_address_t *dest, net_packet_t *packet)
{
    net_iface_t *iface;
    u32 len = net_packet_get_len(packet);
    UNUSED(src);

    /* Determine interface over which packet is to be sent */
    iface = route_get_iface(dest);
    if(iface == NULL)
        return EHOSTUNREACH;

    dev_t * const dev = iface->dev;

    ++iface->stats.tx_packets;
    iface->stats.tx_bytes += len;

    return dev->write(dev, 0, &len, net_packet_get_start(packet));
}


/*
    net_tx_free() - send a packet, then free the packet.
*/
s32 net_tx_free(const net_address_t *src, const net_address_t *dest, net_packet_t *packet)
{
    ks32 ret = net_tx(src, dest, packet);

    net_packet_free(packet);

    return ret;
}


/*
    net_receive() - handle incoming packets on an interface.
    NOTE: this function runs as a kernel process, one per interface.
*/
void net_receive(void *arg)
{
    s32 ret;
    net_iface_t * const iface = (net_iface_t *) arg;
    volatile net_iface_t * const iface_v = (volatile net_iface_t *) arg;
    net_packet_t *packet;

    /* FIXME - get MTU from interface and use as packet size here */
    if(net_packet_alloc_raw(1500, &packet) != SUCCESS)
    {
        kernel_warning("Failed to allocate packet buffer");
        return;
    }

    packet->iface = iface;

    while(1)
    {
        /* TODO - ensure that the interface is configured before calling eth_handle_packet() */
        packet->proto   = iface->driver->proto;
        net_packet_reset(packet);   /* FIXME - not sure it is right to set len to 1500 here; instead raw.len should be reset? */
        ret = iface->dev->read(iface->dev, 0, &packet->len, packet->raw.data);

        /* TODO - this assumes we're working with an Ethernet interface - genericise */
        if(ret == SUCCESS)
        {
            ku32 bytes_received = net_packet_get_len(packet);
            if((eth_rx(packet) == SUCCESS) && (iface_v->proto_addr.type != na_unknown))
            {
                ++iface->stats.rx_packets;
                iface->stats.rx_bytes += bytes_received;
            }
            else
                ++iface->stats.rx_dropped;
        }
    }
}


/*
    net_cksum() - calculate the "Internet checksum" of the specified buffer.
    See RFC 791 & RFC 1071.
*/
s16 net_cksum(const void *buf, u32 len)
{
    u32 x, sum = 0;
    u16 *p;

    if((addr_t) buf & 1)
        kernel_fatal("net_cksum(): supplied buffer is not 2^1-aligned");

    if(len > 65535)
        kernel_fatal("net_cksum(): supplied buffer is >65535 bytes");

    for(p = (u16 *) buf, x = len >> 1; x--;)
        sum += *p++;

    if(len & 1)
        sum += *((u8 *) p) << 8;

    return ~(sum + (sum >> 16));
}


/*
    net_address_compare() - compare two net_address_t objects.  Same semantics as memcmp().
*/
s32 net_address_compare(const net_address_t *a1, const net_address_t *a2)
{
    return memcmp(a1, a2, sizeof(net_address_t));
}


/*
    net_address_get_type() - return the address type of an address.
*/
net_addr_type_t net_address_get_type(const net_address_t * const addr)
{
    return addr->type;
}


/*
    net_address_set_type() - set the type of an address.
*/
s32 net_address_set_type(const net_addr_type_t type, net_address_t * const addr)
{
    if((type < na_unknown) || (type >= na_invalid))
        return EINVAL;

    addr->type = type;
    return SUCCESS;
}


/*
    net_address_get_address() - return a pointer to the network address stored in a net_address_t
    object.
*/
void *net_address_get_address(net_address_t * const addr)
{
    return &addr->addr;
}


/*
    net_address_get_proto() - get the protocol associated with a network address.
*/
net_protocol_t net_address_get_proto(const net_address_t * const addr)
{
    switch(addr->type)
    {
        case na_ethernet:   return np_ethernet;
        case na_ipv4:       return np_ipv4;
        case na_ipv6:       return np_ipv6;

        default:            return np_unknown;
    }
}


/*
    net_print_addr() - print a human-readable form of addr into buf.
*/
s32 net_print_addr(const net_address_t *addr, char *buf, s32 len)
{
    if(addr->type == na_ethernet)
        return eth_print_addr((const mac_addr_t *) &addr->addr.addr, buf, len);
    else if(addr->type == na_ipv4)
        return ipv4_print_addr((const ipv4_addr_t *) &addr->addr.addr, buf, len);
    else
        return -EINVAL;
}
