/*
    Network packet routing functions

    Part of ayumos


    (c) Stuart Wallace, February 2016.
*/

#include <kernel/net/route.h>
#include <kernel/net/arp.h>


/*
    route_get_hw_addr() - get the hardware (currently Ethernet) address corresponding to the
    specified protocol address, performing a lookup (and sleeping) if necessary.
*/
s32 route_get_hw_addr(net_iface_t *iface, const net_address_t *proto_addr, net_address_t *hw_addr)
{
    if(proto_addr->type == na_ipv4)
    {
        arp_cache_item_t *item;

        /* Check ARP cache */
        item = arp_cache_lookup(iface, proto_addr);
        if(item)
        {
            *hw_addr = item->hw_addr;
            return SUCCESS;
        }

        /* Is this a broadcast address? */
        if(*((ipv4_addr_t *) &proto_addr->addr) == IPV4_ADDR_BROADCAST)
        {
            *hw_addr = g_eth_broadcast;
            return SUCCESS;
        }

        /* Attempt to do ARP lookup */
        puts("Can't route yet");

        return EHOSTUNREACH;
    }

    return EHOSTUNREACH;
}
