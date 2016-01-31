/*
    Network interface abstraction

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/device/device.h>
#include <kernel/error.h>
#include <kernel/net/arp.h>
#include <kernel/net/ethernet.h>
#include <kernel/net/net.h>
#include <kernel/process.h>
#include <kernel/util/kutil.h>
#include <klibc/strings.h>
#include <klibc/stdio.h>


s32 net_add_interface(dev_t *dev);
s32 net_handle_packet(net_packet_t *packet);
void net_receive(void *arg);
s32 net_rx_unimplemented(net_packet_t *packet);
s32 net_tx_unimplemented(net_addr_t *src, net_addr_t *dest, buffer_t *payload);
s32 net_reply_unimplemented(net_packet_t *packet, buffer_t *payload);

net_iface_t *g_net_ifaces = NULL;

net_proto_driver_t *g_net_proto_drivers;


/*
    net_init() - initialise networking layer
*/
s32 net_init()
{
    dev_t *dev = NULL;

    /* Register protocols */
    net_register_proto_driver(np_arp, "ARP", arp_init);
    net_register_proto_driver(np_ipv4, "IPv4", ipv4_init);

    /* Iterate over hardware devices; look for any which identify as network interfaces. */
    while((dev = dev_get_next(dev)) != NULL)
        if(dev->type == DEV_TYPE_NET)
        {
            ks32 ret = net_add_interface(dev);

            if(ret != SUCCESS)
                printf("net: failed to add %s: %s\n", dev->name, kstrerror(ret));
        }

    return SUCCESS;
}


/*
    net_register_proto_driver() - register a driver for a particular network protocol
*/
s32 net_register_proto_driver(const net_protocol_t proto, const char * const name,
                              s32 (*init_fn)(net_proto_driver_t *))
{
    s32 ret;
    net_proto_driver_t *driver = (net_proto_driver_t *) CHECKED_KMALLOC(sizeof(net_proto_driver_t)),
                        *p;

    driver->proto = proto;
    driver->name = name;

    driver->rx = net_rx_unimplemented;
    driver->tx = net_tx_unimplemented;
    driver->reply = net_reply_unimplemented;

    driver->next = NULL;

    ret = init_fn(driver);
    if(ret != SUCCESS)
    {
        kfree(driver);
        return ret;
    }

    if(g_net_proto_drivers)
    {
        for(p = g_net_proto_drivers; p->next; p = p->next)
            ;

        p->next = driver;
    }
    else
        g_net_proto_drivers = driver;

    printf("net: registered protocol %s\n", name);
    return SUCCESS;
}


s32 net_rx_unimplemented(net_packet_t *packet)
{
    UNUSED(packet);
    return ENOSYS;
}


s32 net_tx_unimplemented(net_addr_t *src, net_addr_t *dest, buffer_t *payload)
{
    UNUSED(src);
    UNUSED(dest);
    UNUSED(payload);
    return ENOSYS;
}


s32 net_reply_unimplemented(net_packet_t *packet, buffer_t *payload)
{
    UNUSED(packet);
    UNUSED(payload);
    return ENOSYS;
}


/*
    net_add_interface() - add a network interface based on the specified hardware device
*/
s32 net_add_interface(dev_t *dev)
{
    net_iface_t **p, *iface;
    net_addr_type_t addr_type;

    mac_addr_t *ma;
    s32 ret;

    ret = dev->control(dev, dc_get_hw_addr_type, NULL, &addr_type);
    if(ret != SUCCESS)
        return ret;

    if(addr_type == na_ethernet)    /* Ethernet interface */
    {
        iface = (net_iface_t *) CHECKED_KMALLOC(sizeof(net_iface_t));
        iface->next = NULL;
        iface->dev = dev;
        iface->hw_addr.type = addr_type;
        iface->type = ni_ethernet;
        iface->tx = eth_tx;

        bzero(&iface->stats, sizeof(net_iface_stats_t));

        ret = dev->control(dev, dc_get_hw_addr, NULL, &iface->hw_addr.addr);
        if(ret != SUCCESS)
        {
            kfree(iface);
            return ret;
        }

        for(p = &g_net_ifaces; *p != NULL; p = &(*p)->next)
            ;

        *p = iface;

        ma = (mac_addr_t *) &iface->hw_addr.addr;
        printf("net: added %s: %02x:%02x:%02x:%02x:%02x:%02x\n", dev->name,
               ma->b[0], ma->b[1], ma->b[2], ma->b[3], ma->b[4], ma->b[5]);

        /* FIXME - remove (hardwiring IPv4 addr to 172.16.0.200) */
        ipv4_addr_t addr = 0xac1000c8;      /* = 172.16.0.200 */
        iface->proto_addr.type = na_ipv4;
        memcpy(&iface->proto_addr.addr, &addr, sizeof(addr));

        return proc_create(0, 0, "[net_rx]", NULL, net_receive, iface, 0, PROC_TYPE_KERNEL, NULL,
                           NULL);
    }
    else
        return EPROTONOSUPPORT;
}


/*
    net_transmit() - send a packet over an interface.
*/
s32 net_transmit(net_iface_t *iface, const void *buffer, u32 len)
{
    return iface->dev->write(iface->dev, 0, &len, buffer);
}


/*
    net_receive() - handle incoming packets on an interface.
    NOTE: this function runs as a kernel process, one per interface.
*/
void net_receive(void *arg)
{
    s32 ret;
    net_iface_t * const iface = (net_iface_t *) arg;
    net_packet_t packet;

    if(buffer_alloc(1500, &packet.raw) != SUCCESS)
    {
        kernel_warning("Failed to allocate packet buffer");
        return;
    }

    packet.iface = iface;

    while(1)
    {
        /* TODO - ensure that the interface is configured before calling eth_handle_packet() */

        packet.raw->len = 1500;
        ret = iface->dev->read(iface->dev, 0, &packet.raw->len, buffer_dptr(packet.raw));

        /* TODO - this assumes we're working with an Ethernet interface - genericise */
        /* TODO - statistics */
        if(ret == SUCCESS)
        {
            if(eth_identify_proto(&packet) == SUCCESS)
                net_handle_packet(&packet);
        }
    }
}


/*
    net_handle_packet() - invoke a handler appropriate to the protocol of a packet.
*/
s32 net_handle_packet(net_packet_t *packet)
{
    net_proto_driver_t *driver;
    net_iface_stats_t * const stats = &packet->iface->stats;

    ++stats->rx_packets;
    stats->rx_bytes += packet->raw->len;

    for(driver = g_net_proto_drivers; driver; driver = driver->next)
    {
        if(driver->proto == packet->proto)
            return driver->rx(packet);
    }

    ++stats->rx_dropped;

    return EPROTONOSUPPORT;
}


/*
    net_route_get() - look up a route in the kernel routing table.
*/
net_iface_t *net_route_get(const net_addr_type_t addr_type, const net_addr_t *addr)
{
    UNUSED(addr_type);
    UNUSED(addr);

    /* TODO */

    return NULL;
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
