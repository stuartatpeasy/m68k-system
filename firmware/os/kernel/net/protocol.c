/*
    Network protocol abstraction

    Part of ayumos


    (c) Stuart Wallace, May 2016.
*/

#include <kernel/net/protocol.h>


s32 net_protocol_register_driver(s32 (*init_fn)(net_proto_driver_t *));

net_proto_driver_t *g_net_proto_drivers;


/*
    net_protocol_init() - initialise protocol drivers
*/
s32 net_protocol_init()
{
    net_protocol_register_driver(arp_init);
    net_protocol_register_driver(ipv4_init);
    net_protocol_register_driver(eth_init);

}


/*
    net_protocol_register_driver() - register a driver for a particular network protocol
*/
s32 net_protocol_register_driver(s32 (*init_fn)(net_proto_driver_t *))
{
    s32 ret;
    net_proto_driver_t *driver = (net_proto_driver_t *) CHECKED_KMALLOC(sizeof(net_proto_driver_t)),
                        *p;

    driver->rx              = net_rx_unimplemented;
    driver->tx              = net_tx_unimplemented;

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
    net_protocol_get_driver() - look up a protocol driver by protocol
*/
net_proto_driver_t *net_protocol_get_driver(const net_protocol_t proto)
{
    net_proto_driver_t *d;

    for(d = g_net_proto_drivers; d && (d->proto != proto); d = d->next)
        ;

    return d;
}
