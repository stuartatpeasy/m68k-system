/*
    ARP implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/include/net/arp.h>
#include <kernel/include/net/interface.h>
#include <kernel/include/net/ipv4.h>
#include <kernel/include/net/packet.h>
#include <kernel/include/net/protocol.h>
#include <kernel/include/memory/kmalloc.h>
#include <kernel/include/process.h>
#include <klibc/stdlib.h>


arp_cache_item_t *g_arp_cache = NULL;
extern time_t g_current_timestamp;


arp_cache_item_t *arp_cache_get_entry_for_insert();
s32 arp_rx(net_address_t *src, net_address_t *dest, net_packet_t *packet);
s32 arp_send_request(const net_address_t *addr);
s32 arp_packet_alloc(const net_address_t * const addr, ku32 len, net_iface_t *iface,
                     net_packet_t **packet);


/*
    arp_init() - initialise ARP cache
*/
s32 arp_init()
{
    net_proto_fns_t fns;

    if(g_arp_cache != NULL)
        kfree(g_arp_cache);

    /*
        Attempt to allocate ARP cache.  Note that we don't check for errors here: the only possible
        error is ENOMEM, in which case g_arp_cache will be set to NULL (i.e. "cache uninitialised").
        In this case, the cache is disabled but ARP will still work.
    */
    g_arp_cache = kcalloc(ARP_CACHE_SIZE, sizeof(arp_cache_item_t));

    net_proto_fns_struct_init(&fns);

    fns.rx = arp_rx;
    fns.packet_alloc = arp_packet_alloc;

    return net_protocol_register_driver(np_arp, "ARP", &fns);
}


/*
    arp_rx() - handle an incoming ARP packet.  Returns EINVAL if the packet is invalid; returns
    ESUCCESS if the packet was successfully processed, or if the packet was ignored.  Currently
    only supports Ethernet+IP responses.
*/
s32 arp_rx(net_address_t *src, net_address_t *dest, net_packet_t *packet)
{
    arp_hdr_t * const hdr = (arp_hdr_t *) net_packet_get_start(packet);
    arp_payload_t * payload;
    net_address_t dst;
    net_iface_t *iface;
    UNUSED(src);
    UNUSED(dest);

    /* Ensure that a complete header is present, and then verify that the packet is complete */
    if(net_packet_get_len(packet) < sizeof(arp_eth_ipv4_packet_t))
        return EINVAL;      /* Incomplete packet */

    /* Only interested in ARP packets containing Ethernet+IPv4 addresses */
    if(hdr->hw_type != BE2N16(arp_hw_type_ethernet) || hdr->proto_type != BE2N16(ethertype_ipv4))
        return SUCCESS;     /* Discard - not an Ethernet+IPv4 ARP request */

    payload = (arp_payload_t *) &hdr[1];

    ipv4_make_addr(payload->dst_ip, IPV4_PORT_NONE, &dst);

    iface = net_packet_get_interface(packet);

    if((hdr->opcode == BE2N16(arp_request))
       && !net_address_compare(&dst, net_interface_get_proto_addr(iface)))
    {
        /* This is an Ethernet+IPv4 request addressed to this interface; send a response */
        net_address_t hw_addr, proto_addr;

        eth_make_addr(&payload->src_mac, &hw_addr);
        ipv4_make_addr(payload->src_ip, IPV4_PORT_NONE, &proto_addr);

        arp_cache_add(iface, &hw_addr, &proto_addr);

        hdr->opcode = arp_reply;

        payload->dst_ip = payload->src_ip;
        payload->dst_mac = payload->src_mac;

        payload->src_ip = ipv4_get_addr(net_interface_get_proto_addr(iface));
        payload->src_mac = *eth_get_addr(net_interface_get_hw_addr(iface));

        return net_protocol_tx(NULL, &hw_addr, packet);
    }
    else if(hdr->opcode == BE2N16(arp_reply))
    {
        /* This is an Ethernet+IPv4 ARP reply.  Add the data to the cache. */
        net_address_t hw_addr, proto_addr;

        eth_make_addr(&payload->src_mac, &hw_addr);
        ipv4_make_addr(payload->src_ip, IPV4_PORT_NONE, &proto_addr);

        return arp_cache_add(iface, &hw_addr, &proto_addr);
    }

    return SUCCESS;     /* Discard - not an ARP request/response opcode */
}


/*
    arp_packet_alloc() - allocate an packet to hold an ARP request.
*/
s32 arp_packet_alloc(const net_address_t * const addr, ku32 len, net_iface_t *iface,
                     net_packet_t **packet)
{
    ks32 ret = net_protocol_packet_alloc(net_address_get_proto(addr), addr, len, iface, packet);

    if(ret != SUCCESS)
        return ret;

    net_packet_set_proto(*packet, np_arp);

    return SUCCESS;
}


/*
    arp_send_request() - send an ARP request to resolve the specified IPv4 address.
*/
s32 arp_send_request(const net_address_t *addr)
{
    arp_eth_ipv4_packet_t *p;
    net_packet_t *pkt;
    net_iface_t *iface;
    net_address_t src, bcast;
    s32 ret;

    if(net_address_get_type(addr) != na_ipv4)
        return EPROTONOSUPPORT;

    ret = net_route_get_iface(addr, &iface);
    if(ret != SUCCESS)
        return ret;

    if(net_interface_get_proto(iface) != np_ethernet)
        return EPROTONOSUPPORT;

    eth_make_broadcast_addr(&bcast);

    ret = net_protocol_packet_alloc(np_arp, &bcast, sizeof(arp_eth_ipv4_packet_t), iface, &pkt);
    if(ret != SUCCESS)
        return ret;

    p = (arp_eth_ipv4_packet_t *) net_packet_get_start(pkt);

    p->hdr.hw_type          = arp_hw_type_ethernet;
    p->hdr.proto_type       = ethertype_ipv4;
    p->hdr.hw_addr_len      = sizeof(mac_addr_t);
    p->hdr.proto_addr_len   = sizeof(ipv4_addr_t);
    p->hdr.opcode           = arp_request;

    p->payload.src_ip       = ipv4_get_addr(net_interface_get_proto_addr(iface));
    p->payload.src_mac      = *eth_get_addr(net_interface_get_hw_addr(iface));
    p->payload.dst_ip       = ipv4_get_addr(addr);
    p->payload.dst_mac      = *eth_get_addr(&bcast);

    src = *net_interface_get_hw_addr(iface);

    ret = net_protocol_tx(&src, &bcast, pkt);
    net_packet_free(pkt);

    return ret;
}


/*
    arp_lookup() - look up the supplied hardware address on the specified network interface.
    FIXME: arp_lookup() isn't used; maybe remove
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
            ret = arp_send_request(proto_addr);
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
