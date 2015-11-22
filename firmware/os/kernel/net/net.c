/*
    Network interface abstraction

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/device/device.h>
#include <kernel/net/arp.h>
#include <kernel/net/ethernet.h>
#include <kernel/net/net.h>
#include <kernel/process.h>
#include <kernel/util/kutil.h>
#include <stdio.h>


s32 net_add_interface(dev_t *dev);
void net_receive(void *arg);

net_iface_t *g_net_ifaces = NULL;


/*
    net_init() - initialise networking layer
*/
s32 net_init()
{
    dev_t *dev = NULL;
    s32 ret;

    /* Iterate over hardware devices; look for any which identify as network interfaces. */
    while((dev = dev_get_next(dev)) != NULL)
    {
        if(dev->type == DEV_TYPE_NET)
        {
            if(dev->subtype == DEV_SUBTYPE_ETHERNET)
            {
                ks32 ret = net_add_interface(dev);

                if(ret != SUCCESS)
                    printf("net: failed to add %s: %s\n", dev->name, kstrerror(ret));
            }
            else
            {
                printf("net: ignoring unsupported network device %s\n", dev->name);
            }
        }
    }

    ret = arp_init();
    if(ret != SUCCESS)
        printf("net: arp: failed to initialise: %s\n", kstrerror(ret));

    return SUCCESS;
}


/*
    net_add_interface() - add a network interface based on the specified hardware device
*/
s32 net_add_interface(dev_t *dev)
{
    net_iface_t **p, *iface;
    mac_addr_t *ma;
    s32 ret;

    iface = (net_iface_t *) CHECKED_KCALLOC(1, sizeof(net_iface_t));
    iface->dev = dev;

    ret = dev->control(dev, dc_get_hw_addr_type, NULL, &iface->hw_addr_type);
    if(ret != SUCCESS)
    {
        kfree(iface);
        return ret;
    }

    if(iface->hw_addr_type != na_ethernet)
    {
        kfree(iface);
        return EPROTONOSUPPORT;
    }

    ret = dev->control(dev, dc_get_hw_addr, NULL, &iface->hw_addr);
    if(ret != SUCCESS)
    {
        kfree(iface);
        return ret;
    }

    for(p = &g_net_ifaces; *p != NULL; p = &(*p)->next)
        ;

    *p = iface;

    ma = (mac_addr_t *) &iface->hw_addr;
    printf("net: added %s: %02x:%02x:%02x:%02x:%02x:%02x\n", dev->name,
           ma->b[0], ma->b[1], ma->b[2], ma->b[3], ma->b[4], ma->b[5]);

    /* FIXME - remove (hardwiring IPv4 addr to 172.16.0.200) */
    ipv4_addr_t addr = 0xac1000c8;      /* = 172.16.0.200 */
    iface->proto_addr_type = na_ipv4;
    memcpy(&iface->proto_addr, &addr, sizeof(addr));

    proc_create(0, 0, "[net_rx]", NULL, net_receive, iface, 0, PROC_TYPE_KERNEL, NULL, NULL);

    return SUCCESS;
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
    net_iface_t * const iface = (net_iface_t *) arg;
    u8 *buf;
    u32 len = 1500;

    buf = kmalloc(len);

    while(1)
    {
        u32 nread = len;
        s32 ret = iface->dev->read(iface->dev, 0, &nread, buf);

        /* TODO - this assumes we're working with an Ethernet interface - genericise */
        if(ret == SUCCESS)
            eth_handle_packet(iface, buf, nread);
    }
}
