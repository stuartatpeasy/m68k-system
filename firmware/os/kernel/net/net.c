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
#include <kernel/net/packet.h>
#include <kernel/net/raw.h>
#include <kernel/net/route.h>
#include <kernel/process.h>
#include <kernel/util/kutil.h>
#include <klibc/strings.h>
#include <klibc/stdio.h>


net_init_fn_t g_net_init_fns[] =
{
    arp_init,
    ipv4_init,
    eth_init,
    raw_init
};


/*
    net_init() - initialise networking layer
*/
s32 net_init()
{
    u32 i;

    for(i = 0; i < sizeof(g_net_init_fns); ++i)
    {
        ks32 ret = g_net_init_fns[i]();
        if(ret != SUCCESS)
            return ret;
    }

    return net_interface_init();
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
    iface = net_route_get(dest);
    if(iface == NULL)
        return EHOSTUNREACH;

    dev_t * const dev = net_interface_get_device(iface);

    net_interface_stats_inc_tx_packets(iface);
    net_interface_stats_add_tx_bytes(iface, len);

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
    net_iface_t * const iface = (net_iface_t *) arg;
    net_packet_t *packet;

    /* FIXME - get MTU from interface and use as packet size here */
    if(net_protocol_packet_alloc(np_raw, NULL, 1500, iface, &packet) != SUCCESS)
    {
        kernel_warning("Failed to allocate packet buffer");
        return;
    }

    while(1)
        net_interface_rx(iface, packet);
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
