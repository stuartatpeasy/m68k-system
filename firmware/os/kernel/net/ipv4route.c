/*
    IPv4 network packet routing functions

    Part of ayumos


    (c) Stuart Wallace, February 2016.
*/

#include <kernel/net/ipv4route.h>
#include <kernel/net/arp.h>
#include <klibc/string.h>


/* The IPv4 routing table */
ipv4_rt_item_t *g_ipv4_routes = NULL;


/*
    ipv4_route_add() - add an item to the IPv4 routing table.  Fail if the new item is a duplicate
    of an existing item.
*/
s32 ipv4_route_add(const ipv4_route_t * const r)
{
    ipv4_rt_item_t **p;

    /* Walk to the end of the routing table; fail if a duplicate entry exists */
    for(p = &g_ipv4_routes; *p != NULL; p = &(*p)->next)
    {
        const ipv4_route_t * const rt = &(*p)->r;

        if((rt->dest == r->dest) && (rt->mask == r->mask) && (rt->gateway == r->gateway))
            return EEXIST;
    }

    *p = kmalloc(sizeof(ipv4_rt_item_t));
    if(!*p)
        return ENOMEM;

    memcpy(&(*p)->r, r, sizeof(ipv4_route_t));
    (*p)->next = NULL;

    return SUCCESS;

}


/*
    ipv4_route_delete() - delete an item from the IPv4 routing table.  Fail if no matching entry
    exists.
*/
s32 ipv4_route_delete(const ipv4_route_t * const r)
{
    ipv4_rt_item_t **p, *prev;

    /* Walk to the end of the routing table; fail if a duplicate entry exists */
    for(prev = g_ipv4_routes, p = &g_ipv4_routes; *p != NULL; p = &(*p)->next, prev = *p)
    {
        const ipv4_route_t * const rt = &(*p)->r;

        if((rt->dest == r->dest) && (rt->mask == r->mask) && (rt->gateway == r->gateway))
        {
            if(prev)
                prev->next = (*p)->next;
			else
				prev = *p;

            kfree(*p);

            return SUCCESS;
        }
    }

    return ENOENT;
}


/*
    ipv4_route_get_entry() - can be used to iterate over routing table entries
*/
s32 ipv4_route_get_entry(ipv4_rt_item_t **e)
{
    *e = (*e == NULL) ? g_ipv4_routes : (*e)->next;

    return *e ? SUCCESS : ENOENT;
}


/*
    ipv4_route_get_iface() - get the interface associated with a particular address
*/
s32 ipv4_route_get_iface(const net_address_t *proto_addr, net_iface_t **iface)
{
    ipv4_addr_t ipv4_addr;
    ipv4_rt_item_t **p;

    if(proto_addr->type != na_ipv4)
        return EHOSTUNREACH;        /* Should this be EPROTONOSUPPORT? */

    ipv4_addr = ((ipv4_address_t *) &proto_addr->addr)->addr;

    for(p = &g_ipv4_routes; *p != NULL; p = &(*p)->next)
    {
        const ipv4_route_t * const r = &(*p)->r;
        if(((r->dest & r->mask) == (ipv4_addr & r->mask)) && ((r->flags) & IPV4_ROUTE_UP))
        {
            *iface = r->iface;
            return SUCCESS;
        }
    }

    return EHOSTUNREACH;
}


/*
    ipv4_route_get_hw_addr() - get the hardware (currently Ethernet) address corresponding to the
    specified protocol address, performing a lookup (and sleeping) if necessary.
*/
s32 ipv4_route_get_hw_addr(net_iface_t *iface, const net_address_t *proto_addr,
                           net_address_t *hw_addr)
{
    arp_cache_item_t *item;

    if(proto_addr->type != na_ipv4)
        return EHOSTUNREACH;        /* Should this be EPROTONOSUPPORT? */

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
