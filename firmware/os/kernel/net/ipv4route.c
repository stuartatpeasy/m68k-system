/*
    IPv4 network packet routing functions

    Part of ayumos


    (c) Stuart Wallace, February 2016.

    FIXME remove debug log message and #include of stdio
*/

#include <kernel/net/ipv4route.h>
#include <kernel/net/arp.h>
#include <klibc/string.h>
#include <klibc/stdio.h>


/* The IPv4 routing table */
ipv4_rt_item_t *g_ipv4_routes = NULL;

/* A pointer to the default route entry */
ipv4_route_t *ipv4_default_route = NULL;


/*
    ipv4_route_add() - add an item to the IPv4 routing table.  Fail if the new item is a duplicate
    of an existing item.

    FIXME - ipv4_route_add should take route args individually, probably
*/
s32 ipv4_route_add(const ipv4_route_t * const r)
{
    ipv4_rt_item_t **p;
    ipv4_route_t *newent;

    if(!ipv4_mask_valid(r->mask) || (r->metric < 0))
        return EINVAL;

    for(p = &g_ipv4_routes; *p != NULL; p = &(*p)->next)
    {
        const ipv4_route_t * const rt = &((*p)->r);

        /*
            A duplicate is either: a route with a matching destination, gateway and mask, or a route
            with destination and mask equal to 0.0.0.0 where another such route (regardless of
            gateway) already exists.
        */
        if((rt->mask == r->mask) && (rt->dest == r->dest) &&
           ((rt->gateway == r->gateway) || (rt->dest == IPV4_ADDR_NONE)))
            return EEXIST;
    }

    *p = kmalloc(sizeof(ipv4_rt_item_t));
    if(!*p)
        return ENOMEM;

    newent = &((*p)->r);

    memcpy(newent, r, sizeof(ipv4_route_t));

    newent->prefix_len = ipv4_mask_to_prefix_len(r->mask);

    (*p)->next = NULL;

    /* If the new entry is the default route, point ipv4_default_route at it */
    if((r->gateway == IPV4_ADDR_NONE) && (r->mask == IPV4_MASK_NONE))
        ipv4_default_route = newent;

    return SUCCESS;

}


/*
    ipv4_route_delete() - delete an item from the IPv4 routing table.  Fail if no matching entry
    exists.
*/
s32 ipv4_route_delete(const ipv4_route_t * const r)
{
    ipv4_rt_item_t *p, **prev;

    /* Walk to the end of the routing table; fail if a duplicate entry exists */
    for(prev = NULL, p = g_ipv4_routes; p; prev = &p, p = p->next)
    {
        const ipv4_route_t * const rt = &(p->r);

        if((rt->dest == r->dest) && (rt->mask == r->mask) && (rt->gateway == r->gateway))
        {
            /* If the default route is being deleted, unset ipv4_default_route */
            if((r->gateway == IPV4_ADDR_NONE) && (r->mask == IPV4_ADDR_NONE))
                ipv4_default_route = NULL;

            if(*prev)
                (*prev)->next = p->next;
			else
				*prev = p->next;

            kfree(p);

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
    ipv4_route_get() - get a routing table entry corresponding to an address.
*/
const ipv4_route_t *ipv4_route_get(const net_address_t * const proto_addr)
{
    ipv4_addr_t ipv4_addr;
    ipv4_rt_item_t *p;
    s16 best_prefix_len, best_metric;
    const ipv4_route_t *best_route;

    if(net_address_get_type(proto_addr) != na_ipv4)
        return NULL;

    ipv4_addr = ipv4_get_addr(proto_addr);

    best_prefix_len = -1;
    best_metric = -1;
    best_route = ipv4_default_route;

    for(p = g_ipv4_routes; p; p = p->next)
    {
        const ipv4_route_t * const r = &p->r;

        if((r->dest & r->mask) == (ipv4_addr & r->mask) &&
           (r->flags & IPV4_ROUTE_UP) &&
           (r->prefix_len > best_prefix_len) &&
           (r->metric > best_metric))
            {
                best_prefix_len = r->prefix_len;
                best_metric = r->metric;
                best_route = r;
            }
    }

    return best_route;
}


/*
    ipv4_route_get_iface() - get the interface associated with a particular address.  Return NULL
    if no suitable route was found and no default route exists.
*/
net_iface_t *ipv4_route_get_iface(const net_address_t * const proto_addr)
{
    const ipv4_route_t * const route = ipv4_route_get(proto_addr);

    return route ? route->iface : NULL;
}


/*
    ipv4_route_get_hw_addr() - get the hardware (currently Ethernet) address corresponding to the
    specified protocol address, performing a lookup (and sleeping) if necessary.
*/
s32 ipv4_route_get_hw_addr(net_iface_t *iface, const net_address_t *proto_addr,
                           net_address_t *hw_addr)
{
    arp_cache_item_t *item;
    net_address_t ipv4_addr_broadcast;

    if(net_address_get_type(proto_addr) != na_ipv4)
        return EHOSTUNREACH;        /* Should this be EPROTONOSUPPORT? */

    /* Check ARP cache */
    item = arp_cache_lookup(iface, proto_addr);
    if(item)
    {
        *hw_addr = item->hw_addr;
        return SUCCESS;
    }

    /* Is this a broadcast address? */
    if(!ipv4_addr_compare(proto_addr, ipv4_make_broadcast_addr(&ipv4_addr_broadcast)))
    {
        eth_make_broadcast_addr(hw_addr);
        return SUCCESS;
    }

    /* Attempt to do ARP lookup */
    puts("Can't route yet");

    return EHOSTUNREACH;
}
