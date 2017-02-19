/*
    Network interface abstraction

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#ifdef WITH_NETWORKING

#include <kernel/error.h>
#include <kernel/include/defs.h>
#include <kernel/include/device/device.h>
#include <kernel/include/net/arp.h>
#include <kernel/include/net/ethernet.h>
#include <kernel/include/net/icmp.h>
#include <kernel/include/net/interface.h>
#include <kernel/include/net/ipv4.h>
#include <kernel/include/net/net.h>
#include <kernel/include/net/packet.h>
#include <kernel/include/net/raw.h>
#include <kernel/include/net/route.h>
#include <kernel/include/net/tcp.h>
#include <kernel/include/net/udp.h>
#include <kernel/include/process.h>
#include <kernel/util/kutil.h>
#include <klibc/include/strings.h>
#include <klibc/include/stdio.h>


net_init_fn_t g_net_init_fns[] =
{
#ifdef WITH_NET_ARP
    arp_init,
#endif
#ifdef WITH_NET_ICMP
    icmp_init,
#endif
#ifdef WITH_NET_IPV4
    ipv4_init,
#endif
#ifdef WITH_NET_ETHERNET
    eth_init,
#endif
#ifdef WITH_NET_TCP
    tcp_init,
#endif
#ifdef WITH_NET_UDP
    udp_init,
#endif
    raw_init
};


/*
    net_init() - initialise networking layer
*/
s32 net_init()
{
    u32 i, fail;
    net_proto_driver_t *drv;

    fail = 0;
    for(i = 0; i < ARRAY_COUNT(g_net_init_fns); ++i)
    {
        if(g_net_init_fns[i]() != SUCCESS)
            ++fail;
    }

    put("net: registered protocols:");
    for(drv = net_protocol_get_first(); drv; drv = net_protocol_get_next(drv))
        printf(" %s", net_protocol_get_name(drv));

    if(fail)
        printf("; %u protocol(s) failed to register", fail);

    putchar('\n');

    return net_interface_init();
}


/*
    net_tx() - send a packet over an interface.
*/
s32 net_tx(net_packet_t *packet)
{
    net_iface_t *iface;
    dev_t *dev;
    u32 len;

    len = net_packet_get_len(packet);

    /* Routing must be complete by the time this function is called */
    iface = net_packet_get_interface(packet);
    if(iface == NULL)
        return -EHOSTUNREACH;

    /* The packet protocol must match the interface protocol */
    if(net_packet_get_proto(packet) != net_interface_get_proto(iface))
        return -EPROTONOSUPPORT;

    dev = net_interface_get_device(iface);

    net_interface_stats_inc_tx_packets(iface);
    net_interface_stats_add_tx_bytes(iface, len);

    return dev->write(dev, 0, &len, net_packet_get_start(packet));
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

#endif /* WITH_NETWORKING */
