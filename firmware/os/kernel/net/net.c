/*
    Network interface abstraction

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/device/device.h>
#include <kernel/error.h>
#include <kernel/net/arp.h>
#include <kernel/net/ethernet.h>
#include <kernel/net/ipv4.h>
#include <kernel/net/net.h>
#include <kernel/process.h>
#include <kernel/util/kutil.h>
#include <klibc/strings.h>
#include <klibc/stdio.h>


s32 net_add_interface(dev_t *dev);
void net_receive(void *arg);
s32 net_rx_unimplemented(net_packet_t *packet);
s32 net_tx_unimplemented(const net_address_t *src, const net_address_t *dest, net_packet_t *packet);
s32 net_reply_unimplemented(net_packet_t *packet);
s32 net_alloc_packet_unimplemented(net_iface_t *iface, ku32 len, net_packet_t **packet);

net_iface_t *g_net_ifaces = NULL;

net_proto_driver_t *g_net_proto_drivers;


/*
    net_init() - initialise networking layer
*/
s32 net_init()
{
    dev_t *dev = NULL;

    /* Register protocols */
    net_register_proto_driver(arp_init);
    net_register_proto_driver(ipv4_init);
    net_register_proto_driver(eth_init);

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
s32 net_register_proto_driver(s32 (*init_fn)(net_proto_driver_t *))
{
    s32 ret;
    net_proto_driver_t *driver = (net_proto_driver_t *) CHECKED_KMALLOC(sizeof(net_proto_driver_t)),
                        *p;

    driver->rx              = net_rx_unimplemented;
    driver->tx              = net_tx_unimplemented;
    driver->reply           = net_reply_unimplemented;
    driver->alloc_packet    = net_alloc_packet_unimplemented;

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

    printf("net: registered protocol %s\n", driver->name);
    return SUCCESS;
}


/*
    net_get_proto_driver() - look up a protocol driver by protocol
*/
net_proto_driver_t *net_get_proto_driver(const net_protocol_t proto)
{
    net_proto_driver_t *d;

    for(d = g_net_proto_drivers; d && (d->proto != proto); d = d->next)
        ;

    return d;
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


s32 net_reply_unimplemented(net_packet_t *packet)
{
    UNUSED(packet);
    return ENOSYS;
}


s32 net_alloc_packet_unimplemented(net_iface_t *iface, ku32 len, net_packet_t **packet)
{
    UNUSED(iface);
    UNUSED(len);
    UNUSED(packet);
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
        iface->driver = net_get_proto_driver(np_ethernet);

        if(!iface->driver)
        {
            kfree(iface);
            return EPROTONOSUPPORT;
        }

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
    net_get_iface_by_dev() - look up a network interface by device name.
*/
net_iface_t *net_get_iface_by_dev(const char * const name)
{
    net_iface_t **p;

    for(p = &g_net_ifaces; *p != NULL; p = &(*p)->next)
        if(!strcmp((*p)->dev->name, name))
            return *p;

    return NULL;
}


/*
    net_alloc_packet() - allocate a packet object and allocate a buffer of the specified length for
    the payload.
*/
s32 net_alloc_packet(ku32 len, net_packet_t **packet)
{
    s32 ret;
    net_packet_t *p = CHECKED_KMALLOC(sizeof(net_packet_t));

    ret = buffer_init(len, &p->raw);
    if(ret != SUCCESS)
    {
        kfree(p);
        return ret;
    }

    p->start = p->raw.data;
    p->proto = np_unknown;
    p->len = 0;

    *packet = p;

    return SUCCESS;
}


/*
    net_free_packet() - destroy an object created by net_alloc_packet().
*/
void net_free_packet(net_packet_t *packet)
{
    buffer_deinit(&packet->raw);
    kfree(packet);
}


/*
    net_transmit() - send a packet over an interface.
*/
s32 net_transmit(net_packet_t *packet)
{
    u32 len = packet->len;
    dev_t * const dev = packet->iface->dev;

    ++packet->iface->stats.tx_packets;
    packet->iface->stats.tx_bytes += len;

    return dev->write(dev, 0, &len, packet->start);
}


/*
    net_receive() - handle incoming packets on an interface.
    NOTE: this function runs as a kernel process, one per interface.
*/
void net_receive(void *arg)
{
    s32 ret;
    net_iface_t * const iface = (net_iface_t *) arg;
    net_packet_t *packet;

    if(net_alloc_packet(1500, &packet) != SUCCESS)
    {
        kernel_warning("Failed to allocate packet buffer");
        return;
    }

    packet->iface = iface;

    while(1)
    {
        /* TODO - ensure that the interface is configured before calling eth_handle_packet() */
        packet->driver  = iface->driver;
        packet->proto   = iface->driver->proto;
        packet->start   = packet->raw.data;
        packet->raw.len = 1500;
        ret = iface->dev->read(iface->dev, 0, &packet->raw.len, packet->raw.data);

        /* TODO - this assumes we're working with an Ethernet interface - genericise */
        if(ret == SUCCESS)
        {
            packet->len = packet->raw.len;

            if(eth_rx(packet) == SUCCESS)
            {
                ++iface->stats.rx_packets;
                iface->stats.rx_bytes += packet->raw.len;
            }
            else
                ++iface->stats.rx_dropped;
        }
    }
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


/*
    net_address_compare() - compare two net_address_t objects.  Same semantics as memcmp().
*/
s32 net_address_compare(const net_address_t *a1, const net_address_t *a2)
{
    return memcmp(a1, a2, sizeof(net_address_t));
}


/*
    net_print_addr() - print a human-readable form of addr into buf.
*/
s32 net_print_addr(const net_address_t *addr, char *buf, s32 len)
{
    if(addr->type == na_ethernet)
        return eth_print_addr((const mac_addr_t *) &addr->addr.addr, buf, len);
    else if(addr->type == na_ipv4)
        return ipv4_print_addr(addr->addr.addr, buf, len);
    else
        return -EINVAL;
}
