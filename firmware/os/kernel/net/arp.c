/*
    ARP implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/arp.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/process.h>
#include <klibc/stdio.h>            // FIXME remove
#include <klibc/stdlib.h>


arp_cache_item_t *g_arp_cache = NULL;
extern time_t g_current_timestamp;

s32 arp_cache_add(const net_iface_t * const iface, const net_address_t *hw_addr,
                  const net_address_t *proto_addr);
arp_cache_item_t *arp_cache_get_entry_for_insert();
s32 arp_send_request(net_iface_t *iface, const net_address_t *addr);


/*
    arp_init() - initialise ARP cache
*/
s32 arp_init(net_proto_driver_t *driver)
{
    if(g_arp_cache != NULL)
        kfree(g_arp_cache);

    /*
        Attempt to allocate ARP cache.  Note that we don't check for errors here: the only possible
        error is ENOMEM, in which case g_arp_cache will be set to NULL (i.e. "cache uninitialised").
        In this case, the cache is disabled but ARP will still work.
    */
    g_arp_cache = kcalloc(ARP_CACHE_SIZE, sizeof(arp_cache_item_t));

    driver->name = "ARP";
    driver->proto = np_arp;
    driver->rx = arp_rx;

    return SUCCESS;
}


/*
    arp_rx() - handle an incoming ARP packet.  Returns EINVAL if the packet is invalid; returns
    ESUCCESS if the packet was successfully processed, or if the packet was ignored.  Currently
    only supports Ethernet+IP responses.
*/
s32 arp_rx(net_packet_t *packet)
{
    arp_hdr_t * const hdr = (arp_hdr_t *) packet->start;
    arp_payload_t * payload;

    /* Ensure that a complete header is present, and then verify that the packet is complete */
    if(packet->len < sizeof(arp_hdr_t))
        return EINVAL;      /* Incomplete packet */

    /* Only interested in ARP packets containing Ethernet+IPv4 addresses */
    if(hdr->hw_type != BE2N16(arp_hw_type_ethernet) || hdr->proto_type != BE2N16(ethertype_ipv4))
        return SUCCESS;     /* Discard - not an Ethernet+IPv4 ARP request */

    packet->start += sizeof(arp_hdr_t);
    packet->len -= sizeof(arp_hdr_t);

    payload = (arp_payload_t *) packet->start;

    /* Check that we have a full ARP payload */
    if(packet->len < sizeof(arp_payload_t))
        return EINVAL;      /* Incomplete packet */

    if(hdr->opcode == BE2N16(arp_request)
       && payload->dst_ip == *((ipv4_addr_t *) &packet->iface->proto_addr.addr))
    {
        /* This is an Ethernet+IPv4 request addressed to this interface; send a response */
        net_address_t hw_addr, proto_addr;

        eth_make_addr(&payload->src_mac, &hw_addr);
        ipv4_make_addr(payload->src_ip, 0, &proto_addr);

        arp_cache_add(packet->iface, &hw_addr, &proto_addr);

        hdr->opcode = arp_reply;

        payload->dst_ip = payload->src_ip;
        payload->dst_mac = payload->src_mac;

        payload->src_ip = *((ipv4_addr_t *) &packet->iface->proto_addr.addr);
        payload->src_mac = *((mac_addr_t *) &packet->iface->hw_addr.addr);

        return packet->driver->tx(NULL, &hw_addr, packet);
    }
    else if(hdr->opcode == BE2N16(arp_reply))
    {
        /* This is an Ethernet+IPv4 ARP reply.  Add the data to the cache. */
        net_address_t hw_addr, proto_addr;

        eth_make_addr(&payload->src_mac, &hw_addr);
        ipv4_make_addr(payload->src_ip, IPV4_PORT_NONE, &proto_addr);

        return arp_cache_add(packet->iface, &hw_addr, &proto_addr);
    }

    return SUCCESS;     /* Discard - not an ARP request/response opcode */
}


/*
    arp_send_request() - send an ARP request, to resolve the specified IPv4 address, over the
    specified interface.
*/
s32 arp_send_request(net_iface_t *iface, const net_address_t *addr)
{
    s32 ret;

    if((iface->driver->proto == np_ethernet) && (addr->type == na_ipv4))
    {
        arp_eth_ipv4_packet_t *p;
        net_packet_t *pkt;

        ret = iface->driver->packet_alloc(iface, sizeof(arp_eth_ipv4_packet_t), &pkt);
        if(ret != SUCCESS)
            return ret;

        pkt->proto = np_arp;
        pkt->len += sizeof(arp_eth_ipv4_packet_t);

        p = (arp_eth_ipv4_packet_t *) pkt->start;

        p->hdr.hw_type          = arp_hw_type_ethernet;
        p->hdr.proto_type       = ethertype_ipv4;
        p->hdr.hw_addr_len      = sizeof(mac_addr_t);
        p->hdr.proto_addr_len   = sizeof(ipv4_addr_t);
        p->hdr.opcode           = arp_request;

        p->payload.src_ip       = *((ipv4_addr_t *) &iface->proto_addr.addr);
        p->payload.src_mac      = *((mac_addr_t *) &iface->hw_addr.addr);
        p->payload.dst_ip       = *((ipv4_addr_t *) &addr->addr);
        p->payload.dst_mac      = *((mac_addr_t *) &g_eth_broadcast.addr);

        ret = iface->driver->tx(NULL, &g_eth_broadcast, pkt);

        net_packet_free(pkt);

        return ret;
    }

    return EPROTONOSUPPORT;
}


/*
    arp_lookup() - look up the supplied hardware address on the specified network interface.
*/
s32 arp_lookup(net_iface_t *iface, const net_address_t *proto_addr, net_address_t *hw_addr)
{
    arp_cache_item_t *p;
    s32 i, ret;

    for(p = NULL, i = 0; (p == NULL) && (i < ARP_MAX_REQUESTS); ++i)
    {
        p = arp_cache_lookup(iface, proto_addr);
        if(p == NULL)
        {
            /* Address not present in cache; try to look it up instead. */
            ret = arp_send_request(iface, proto_addr);
            if(ret != SUCCESS)
                return ret;

            proc_sleep_for(ARP_REQUEST_INTERVAL);
        }
    }

    if(p == NULL)
        return ENOENT;

    *hw_addr = p->hw_addr;
    return SUCCESS;
}


/*
    arp_cache_lookup() - find an entry in the ARP cache.
*/
arp_cache_item_t *arp_cache_lookup(const net_iface_t * const iface, const net_address_t *proto_addr)
{
    arp_cache_item_t *p;

    if(g_arp_cache != NULL)
    {
        /* Search cache for the requested address */
        for(p = g_arp_cache; p < (g_arp_cache + ARP_CACHE_SIZE); ++p)
            if((p->iface == iface) && !net_address_compare(proto_addr, &p->proto_addr)
               && (p->etime > g_current_timestamp))
                return p;
    }

    return NULL;
}


/*
    arp_cache_add() - add a MAC address / IPv4 address pair to the ARP cache
	Note: returns SUCCESS if the ARP cache is disabled.
*/
s32 arp_cache_add(const net_iface_t * const iface, const net_address_t *hw_addr,
                  const net_address_t *proto_addr)
{
    /* TODO - obtain lock on ARP cache during cache-add operation */
	arp_cache_item_t *p = arp_cache_lookup(iface, proto_addr);

	if(!p)
	{
		/* Cache miss - add entry */
	    p = arp_cache_get_entry_for_insert();

		if(p)
        {
		    p->iface        = iface;
		    p->etime        = g_current_timestamp + ARP_CACHE_ITEM_LIFETIME;
		    p->hw_addr      = *hw_addr;
		    p->proto_addr   = *proto_addr;
		}
	}
	else
	{
		/* Cache hit - update expiry time of entry */
		p->etime = g_current_timestamp + ARP_CACHE_ITEM_LIFETIME;
	}

    return SUCCESS;
}


/*
    arp_cache_get_entry_for_insert() - return a ptr to an ARP cache entry which can be overwritten
    with a new entry.  If the cache is full, returns the item expiring soonest.  Returns NULL if the
    ARP cache is disabled or uninitialised.
*/
arp_cache_item_t *arp_cache_get_entry_for_insert()
{
    arp_cache_item_t *p, *expiring_soonest;
    time_t shortest_expiry;

    if(g_arp_cache == NULL)
        return NULL;

    for(shortest_expiry = TIME_T_MAX, expiring_soonest = NULL, p = g_arp_cache;
        p < (g_arp_cache + ARP_CACHE_SIZE); ++p)
    {
        /* Is the item either unused or expired? */
        if((p->etime <= g_current_timestamp) || (p->iface == NULL))
            return p;

        if(p->etime < shortest_expiry)
        {
            shortest_expiry = p->etime;
            expiring_soonest = p;
        }
    }

    /* No unused/expired items found; return the one that expires soonest */
    return expiring_soonest;
}


/*
    arp_cache_get_item() - return the specified ARP cache entry.
*/
arp_cache_item_t *arp_cache_get_item(ku32 n)
{
    if((g_arp_cache == NULL) || (n >= ARP_CACHE_SIZE))
        return NULL;

    return g_arp_cache + n;
}
