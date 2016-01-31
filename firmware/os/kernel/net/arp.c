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

s32 arp_rx(net_packet_t *packet);
s32 arp_cache_add(const net_iface_t * const iface, const mac_addr_t hw_addr, const ipv4_addr_t ip);
arp_cache_item_t *arp_cache_lookup(const net_iface_t * const iface, const ipv4_addr_t ip);
arp_cache_item_t *arp_cache_get_entry_for_insert();


/*
    arp_init() - initialise ARP cache
*/
s32 arp_init(net_proto_driver_t *driver)
{
    if(g_arp_cache != NULL)
        kfree(g_arp_cache);

    g_arp_cache = kcalloc(ARP_CACHE_SIZE, sizeof(arp_cache_item_t));
    if(g_arp_cache == NULL)
        return ENOMEM;

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
    arp_hdr_t * const hdr = (arp_hdr_t *) packet->payload;
    arp_payload_t * const payload = (arp_payload_t *) &hdr[1];

    /* Ensure that a complete header is present, and then verify that the packet is complete */
    if(packet->payload_len < sizeof(arp_hdr_t))
        return EINVAL;      /* Incomplete packet */

    /* Only interested in ARP packets containing Ethernet+IPv4 addresses */
    if(hdr->hw_type != BE2N16(arp_hw_type_ethernet) || hdr->proto_type != BE2N16(ethertype_ipv4))
        return SUCCESS;     /* Discard - not an Ethernet+IPv4 ARP request */

    /*
        Ensure that the interface protocol is IPv4, and it is configured (i.e. it has a non-zero
        IPv4 address)
    */
    if(packet->iface->proto_addr.type != na_ipv4)
        return SUCCESS;     /* Discard - inappropriate protocol type */

    /* Check that we have a full ARP payload */
    if(packet->payload_len < (sizeof(arp_hdr_t) + sizeof(arp_payload_t)))
        return EINVAL;      /* Incomplete packet */

    if(hdr->opcode == BE2N16(arp_request)
       && payload->dst_ip == *((ipv4_addr_t *) &packet->iface->proto_addr.addr))
    {
        /* This is an Ethernet+IPv4 request addressed to this interface */
        arp_cache_add(packet->iface, payload->src_mac, payload->src_ip);

        hdr->opcode = arp_reply;

        payload->dst_ip = payload->src_ip;
        payload->dst_mac = payload->src_mac;

        payload->src_ip = *((ipv4_addr_t *) &packet->iface->proto_addr.addr);
        payload->src_mac = *((mac_addr_t *) &packet->iface->hw_addr.addr);

        return eth_reply(packet);

    }
    else if(hdr->opcode == BE2N16(arp_reply))
    {
        /* This is an Ethernet+IPv4 ARP reply.  Add the data to the cache. */
        return arp_cache_add(packet->iface, payload->src_mac, payload->src_ip);
    }

    return SUCCESS;     /* Discard - not an ARP request/response opcode */
}


/*
    arp_send_request() - send an ARP request, to resolve the specified IPv4 address, over the
    specified interface.
*/
s32 arp_send_request(net_iface_t *iface, const ipv4_addr_t ip)
{
    UNUSED(iface);
    UNUSED(ip);
/*
    buffer_t *packet;
    arp_eth_ipv4_packet_t *p;
    s32 ret;

    ret = buffer_alloc(sizeof(arp_eth_ipv4_packet_t), &packet);
    if(ret != SUCCESS)
        return ret;

    p = (arp_eth_ipv4_packet_t *) buffer_dptr(packet);

    p->hdr.hw_type          = arp_hw_type_ethernet;
    p->hdr.proto_type       = ethertype_ipv4;
    p->hdr.hw_addr_len      = sizeof(mac_addr_t);
    p->hdr.proto_addr_len   = sizeof(ipv4_addr_t);
    p->hdr.opcode           = arp_request;

    p->payload.src_ip       = *((ipv4_addr_t *) &iface->proto_addr);
    p->payload.src_mac      = *((mac_addr_t *) &iface->hw_addr);
    p->payload.dst_ip       = ip;
    p->payload.dst_mac      = g_mac_broadcast;

    return eth_transmit(iface, &p->payload.dst_mac, ethertype_arp, packet);
*/
    return SUCCESS;
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
