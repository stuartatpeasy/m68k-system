/*
    Network interface abstraction

    Part of ayumos


    (c) Stuart Wallace, May 2016.

    FIXME - replace printf() with kernel logging call of some sort, and remove #include stdio
*/

#include <kernel/device/device.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/net/interface.h>
#include <kernel/net/packet.h>
#include <kernel/net/net.h>
#include <kernel/process.h>
#include <klibc/stdio.h>        // FIXME remove
#include <klibc/strings.h>


s32 net_interface_add(dev_t *dev);

net_iface_t *g_net_ifaces = NULL;

/* A network interface object */
struct net_iface
{
    net_iface_t *       next;
    dev_t *             dev;            /* The hw device implementing this interface            */
    net_protocol_t      proto;          /* Interface protocol (e.g. np_ethernet -> Ethernet)    */
    net_address_t       hw_addr;        /* Hardware address                                     */
    net_address_t       proto_addr;     /* Protocol address                                     */
    net_iface_stats_t   stats;
};


/*
    net_interface_init() - detect and initialise network interfaces
*/
s32 net_interface_init()
{
    dev_t *dev = NULL;

    /* Iterate over hardware devices; look for any which identify as network interfaces. */
    while((dev = dev_get_next(dev)) != NULL)
        if(dev->type == DEV_TYPE_NET)
        {
            ks32 ret = net_interface_add(dev);

            if(ret != SUCCESS)
                printf("net: failed to add %s: %s\n", dev->name, kstrerror(ret));
        }

    return SUCCESS;
}


/*
    net_interface_add() - add a network interface based on the specified hardware device
*/
s32 net_interface_add(dev_t *dev)
{
    net_iface_t **p, *iface;
    net_protocol_t proto;
    char buf[64];
    s32 ret;

    ret = dev->control(dev, dc_get_hw_protocol, NULL, &proto);
    if(ret != SUCCESS)
        return ret;

    iface = (net_iface_t *) CHECKED_KMALLOC(sizeof(net_iface_t));
    iface->next         = NULL;
    iface->dev          = dev;
    iface->hw_addr.type = net_address_type_from_proto(proto);
    iface->proto        = proto;

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
    iface->proto_addr.type = na_unknown;

    net_address_print(&iface->hw_addr, buf, sizeof(buf));
    printf("net: added %s: %s\n", dev->name, buf);

    return proc_create(0, 0, "[net_rx]", NULL, net_receive, iface, 0, PROC_TYPE_KERNEL, NULL,
                       NULL);
}


/*
    net_interface_rx() - receive a packet from an interface.  Will not read more data than the
    packet can accommodate.  May block in call to dev->read().  Updates interface statistics.
*/
s32 net_interface_rx(net_iface_t * const iface, net_packet_t *packet)
{
    u32 len;
    net_protocol_t proto;
    net_address_t src, dest;
    dev_t *dev;
    s32 ret;

    dev = net_interface_get_device(iface);
    len = net_packet_get_buffer_len(packet);

    net_packet_reset(packet);

    ret = dev->read(dev, 0, &len, net_packet_get_start(packet));
    if(ret != SUCCESS)
        return ret;

    proto = net_interface_get_proto(iface);

    net_packet_set_interface(packet, iface);
    net_packet_set_proto(packet, proto);
    net_packet_set_len(packet, len);
    net_interface_stats_add_rx_bytes(iface, len);

    net_address_set_type(np_unknown, &src);
    net_address_set_type(np_unknown, &dest);

    ret = net_protocol_rx(&src, &dest, packet);
    if(ret == SUCCESS)
        net_interface_stats_inc_rx_packets(iface);
    else
    {
        if(ret == ECKSUM)
            net_interface_stats_inc_cksum_err(iface);

        net_interface_stats_inc_rx_dropped(iface);
    }

    return ret;
}


/*
    net_get_iface_name() - get the device name associated with an interface
*/
const char *net_get_iface_name(const net_iface_t * const iface)
{
    return iface->dev->name;
}


/*
    net_interface_get_device() - return the device object associated with an interface
*/
dev_t *net_interface_get_device(net_iface_t * const iface)
{
    return iface->dev;
}


/*
    net_interface_get_proto_addr() - get the protocol address for an interface
*/
const net_address_t *net_interface_get_proto_addr(const net_iface_t * const iface)
{
    return &iface->proto_addr;
}


/*
    net_interface_get_hw_addr() - get the hardware address for an interface
*/
const net_address_t *net_interface_get_hw_addr(const net_iface_t * const iface)
{
    return &iface->hw_addr;
}


/*
    net_interface_set_proto_addr() - set the protocol address for an interface
*/
s32 net_interface_set_proto_addr(net_iface_t * const iface, const net_address_t * const addr)
{
    iface->proto_addr = *addr;
    return SUCCESS;
}


/*
    net_interface_get_by_dev() - look up a network interface by device name.
*/
net_iface_t *net_interface_get_by_dev(const char * const name)
{
    net_iface_t **p;

    for(p = &g_net_ifaces; *p != NULL; p = &(*p)->next)
        if(!strcmp((*p)->dev->name, name))
            return *p;

    return NULL;
}


/*
    net_interface_get_proto() - get the network protocol implemented by a network interface
*/
net_protocol_t net_interface_get_proto(const net_iface_t * const iface)
{
    return iface->proto;
}


/*
    net_interface_stats_inc_rx_packets() - increment the "received packets" count on an interface
*/
void net_interface_stats_inc_rx_packets(net_iface_t * const iface)
{
    ++iface->stats.rx_packets;
}


/*
    net_interface_stats_add_rx_bytes() - add to the "received bytes" count for an interface
*/
void net_interface_stats_add_rx_bytes(net_iface_t * const iface, ku32 bytes)
{
    iface->stats.rx_bytes += bytes;
}


/*
    net_interface_stats_inc_tx_packets() - increment the "transmitted packets" count on an interface
*/
void net_interface_stats_inc_tx_packets(net_iface_t * const iface)
{
    ++iface->stats.tx_packets;
}


/*
    net_interface_stats_add_tx_bytes() - add to the "transmitted bytes" count for an interface
*/
void net_interface_stats_add_tx_bytes(net_iface_t * const iface, ku32 bytes)
{
    iface->stats.tx_bytes += bytes;
}


/*
    net_interface_stats_inc_cksum_err() - increment the "checksum error" count on an interface.
*/
void net_interface_stats_inc_cksum_err(net_iface_t * const iface)
{
    ++iface->stats.rx_checksum_err;
}


/*
    net_interface_stats_inc_rx_dropped() - increment the "received packets dropped" count on an
    interface.
*/
void net_interface_stats_inc_rx_dropped(net_iface_t * const iface)
{
    ++iface->stats.rx_dropped;
}


/*
    net_interface_get_stats() - get a ptr to the net_iface_stats_t object containing the statistics
    associated with an interface.
*/
const net_iface_stats_t * net_interface_get_stats(net_iface_t * const iface)
{
    return &iface->stats;
}
