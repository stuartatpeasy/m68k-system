/*
    ARP implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/arp.h>
#include <kernel/net/ethernet.h>
#include <kernel/memory/kmalloc.h>
#include <kernel/process.h>
#include <klibc/stdlib.h>


arp_cache_item_t *g_arp_cache = NULL;
extern time_t g_current_timestamp;

s32 arp_cache_add(const net_iface_t * const iface, const mac_addr_t hw_addr, const ipv4_addr_t ip);
arp_cache_item_t *arp_cache_lookup(const net_iface_t * const iface, const ipv4_addr_t ip);
arp_cache_item_t *arp_cache_get_entry_for_insert();


/*
    arp_init() - initialise ARP cache
*/
s32 arp_init()
{
    if(g_arp_cache != NULL)
        kfree(g_arp_cache);

    g_arp_cache = kcalloc(ARP_CACHE_SIZE, sizeof(arp_cache_item_t));
    if(g_arp_cache == NULL)
        return ENOMEM;

    return SUCCESS;
}


/*
    arp_handle_packet() - handle an incoming ARP packet.  Return EINVAL if the packet is invalid;
    returns ESUCCESS if the packet was successfully processed, or if the packet was ignored.
    Currently only supports Ethernet+IP responses.
*/
s32 arp_handle_packet(net_iface_t *iface, const void * const packet, u32 len)
{
    const arp_hdr_t * const hdr = (arp_hdr_t *) packet;
    const arp_payload_t * const payload = (arp_payload_t *) ((u8 *) packet + sizeof(arp_hdr_t));

    /* Ensure that a complete header is present, and then verify that the packet is complete */
    if(len < sizeof(arp_hdr_t))
        return EINVAL;      /* Incomplete packet */

    /* Only interested in ARP packets containing Ethernet+IPv4 addresses */
    if(hdr->hw_type != BE2N16(arp_hw_type_ethernet) || hdr->proto_type != BE2N16(ethertype_ipv4))
        return SUCCESS;     /* Discard - not an Ethernet+IPv4 ARP request */

    /*
        Ensure that the interface protocol is IPv4, and it is configured (i.e. it has a non-zero
        IPv4 address)
    */
    if((iface->proto_addr_type != na_ipv4) || !*((ipv4_addr_t *) &iface->proto_addr))
        return SUCCESS;     /* Discard - inappropriate packet type, or IPv4 unconfigured */

    /* Check that we have a full ARP payload */
    if(len < (sizeof(arp_hdr_t) + sizeof(arp_payload_t)))
        return EINVAL;      /* Incomplete packet */

    if(hdr->opcode == BE2N16(arp_request)
       && payload->dst_ip == *((ipv4_addr_t *) &iface->proto_addr))
    {
        /* This is an Ethernet+IPv4 request addressed to this interface */
        arp_eth_ipv4_packet_t p;
        u32 len = sizeof(p);

        arp_cache_add(iface, payload->src_mac, payload->src_ip);

        p.hdr.hw_type           = arp_hw_type_ethernet;
        p.hdr.proto_type        = ethertype_ipv4;
        p.hdr.hw_addr_len       = sizeof(mac_addr_t);
        p.hdr.proto_addr_len    = sizeof(ipv4_addr_t);
        p.hdr.opcode            = arp_reply;

        p.payload.dst_ip        = payload->src_ip;
        p.payload.dst_mac       = payload->src_mac;
        p.payload.src_ip        = *((ipv4_addr_t *) &iface->proto_addr);
        p.payload.src_mac       = *((mac_addr_t *) &iface->hw_addr);

        return eth_transmit(iface, &p.payload.dst_mac, ethertype_arp, &p, len);
    }
    else if(hdr->opcode == BE2N16(arp_reply))
    {
        /* This is an Ethernet+IPv4 ARP reply.  Add the data to the cache. */
        return arp_cache_add(iface, payload->src_mac, payload->src_ip);
    }

    return SUCCESS;     /* Discard - not an ARP request/response opcode */
}


/*
    arp_send_request() - send an ARP request, to resolve the specified IPv4 address, over the
    specified interface.
*/
s32 arp_send_request(net_iface_t *iface, const ipv4_addr_t ip)
{
    arp_eth_ipv4_packet_t p;
    u32 len = sizeof(p);

    p.hdr.hw_type           = arp_hw_type_ethernet;
    p.hdr.proto_type        = ethertype_ipv4;
    p.hdr.hw_addr_len       = sizeof(mac_addr_t);
    p.hdr.proto_addr_len    = sizeof(ipv4_addr_t);
    p.hdr.opcode            = arp_request;

    p.payload.src_ip        = *((ipv4_addr_t *) &iface->proto_addr);
    p.payload.src_mac       = *((mac_addr_t *) &iface->hw_addr);
    p.payload.dst_ip        = ip;
    p.payload.dst_mac       = g_mac_broadcast;

    return eth_transmit(iface, &p.payload.dst_mac, ethertype_arp, &p, len);
}


/*
    arp_lookup_ip() - look up the supplied IPv4 address on the specified network interface.
*/
s32 arp_lookup_ip(const ipv4_addr_t ip, net_iface_t *iface, mac_addr_t *hw_addr)
{
    arp_cache_item_t *p;
    s32 i, ret;

    for(p = NULL, i = 0; (p == NULL) && (i < ARP_MAX_REQUESTS); ++i)
    {
        p = arp_cache_lookup(iface, ip);
        if(p == NULL)
        {
            /* Address not present in cache; try to look it up instead. */
            ret = arp_send_request(iface, ip);
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
arp_cache_item_t *arp_cache_lookup(const net_iface_t * const iface, const ipv4_addr_t ip)
{
    /* TODO */
    arp_cache_item_t *p;

    if(g_arp_cache != NULL)
    {
        /* Search cache for the requested address */
        for(p = g_arp_cache; p < (g_arp_cache + ARP_CACHE_SIZE); ++p)
            if((p->iface == iface) && (p->ipv4_addr == ip) && (p->etime > g_current_timestamp))
                return p;
    }

    return NULL;
}


/*
    arp_cache_add() - add a MAC address / IPv4 address pair to the ARP cache
*/
s32 arp_cache_add(const net_iface_t * const iface, const mac_addr_t hw_addr, const ipv4_addr_t ip)
{
    UNUSED(hw_addr);
    UNUSED(ip);

    /* TODO - obtain lock on ARP cache during cache-add operation */
    arp_cache_item_t *p = arp_cache_get_entry_for_insert();
    if(p == NULL)
        return SUCCESS;     /* Cache disabled; fail silently */

    p->iface        = iface;
    p->etime        = g_current_timestamp + ARP_CACHE_ITEM_LIFETIME;
    p->hw_addr      = hw_addr;
    p->ipv4_addr    = ip;

    return SUCCESS;
}


/*
    arp_cache_get_entry_for_insert() - return a ptr to an ARP cache entry which can be overwritten
    with a new entry.  Selects an entry at random if the cache is full.  Returns NULL if the ARP
    cache is disabled or uninitialised.
*/
arp_cache_item_t *arp_cache_get_entry_for_insert()
{
    arp_cache_item_t *p;

    if(g_arp_cache == NULL)
        return NULL;

    for(p = g_arp_cache; p < (g_arp_cache + ARP_CACHE_SIZE); ++p)
    {
        /* Is the item either unused or expired? */
        if((p->iface == NULL) || (p->etime <= g_current_timestamp))
            return p;
    }

    /* No unused/expired items found; pick an item at random (=random replacement algorithm) */
    return g_arp_cache + (rand() % ARP_CACHE_SIZE);
}
