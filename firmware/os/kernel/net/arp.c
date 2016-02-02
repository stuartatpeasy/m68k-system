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
s32 arp_cache_add(const net_iface_t * const iface, const net_address_t *hw_addr,
                  const net_address_t *proto_addr);
arp_cache_item_t *arp_cache_lookup(const net_iface_t * const iface,
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

    g_arp_cache = kcalloc(ARP_CACHE_SIZE, sizeof(arp_cache_item_t));
    if(g_arp_cache == NULL)
        return ENOMEM;

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
    arp_hdr_t * const hdr = (arp_hdr_t *) packet->raw.data;
    arp_payload_t * const payload = (arp_payload_t *) &hdr[1];

    /* Ensure that a complete header is present, and then verify that the packet is complete */
    if(packet->raw.len < sizeof(arp_hdr_t))
        return EINVAL;      /* Incomplete packet */

    /* Only interested in ARP packets containing Ethernet+IPv4 addresses */
    if(hdr->hw_type != BE2N16(arp_hw_type_ethernet) || hdr->proto_type != BE2N16(ethertype_ipv4))
        return SUCCESS;     /* Discard - not an Ethernet+IPv4 ARP request */

    /* Check that we have a full ARP payload */
    if(packet->raw.len < (sizeof(arp_hdr_t) + sizeof(arp_payload_t)))
        return EINVAL;      /* Incomplete packet */

    if(hdr->opcode == BE2N16(arp_request)
       && payload->dst_ip == *((ipv4_addr_t *) &packet->iface->proto_addr.addr))
    {
        /* This is an Ethernet+IPv4 request addressed to this interface */
        net_address_t hw_addr, proto_addr;

        eth_make_addr(&payload->src_mac, &hw_addr);
        ipv4_make_addr(payload->src_ip, &proto_addr);

        arp_cache_add(packet->iface, &hw_addr, &proto_addr);

        hdr->opcode = arp_reply;

        payload->dst_ip = payload->src_ip;
        payload->dst_mac = payload->src_mac;

        payload->src_ip = *((ipv4_addr_t *) &packet->iface->proto_addr.addr);
        payload->src_mac = *((mac_addr_t *) &packet->iface->hw_addr.addr);

        return packet->parent->driver->reply(packet->parent);

    }
    else if(hdr->opcode == BE2N16(arp_reply))
    {
        /* This is an Ethernet+IPv4 ARP reply.  Add the data to the cache. */
        net_address_t hw_addr, proto_addr;

        eth_make_addr(&payload->src_mac, &hw_addr);
        ipv4_make_addr(payload->src_ip, &proto_addr);

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
    buffer_t buffer;
    s32 ret;

    if(iface->driver->proto == np_ethernet)
    {
        if(addr->type == na_ipv4)
        {
            arp_eth_ipv4_packet_t *p;

            ret = buffer_init(sizeof(arp_eth_ipv4_packet_t), &buffer);
            if(ret != SUCCESS)
                return ret;

            p = (arp_eth_ipv4_packet_t *) buffer.data;

            p->hdr.hw_type          = arp_hw_type_ethernet;
            p->hdr.proto_type       = ethertype_ipv4;
            p->hdr.hw_addr_len      = sizeof(mac_addr_t);
            p->hdr.proto_addr_len   = sizeof(ipv4_addr_t);
            p->hdr.opcode           = arp_request;

            p->payload.src_ip       = *((ipv4_addr_t *) &iface->proto_addr);
            p->payload.src_mac      = *((mac_addr_t *) &iface->hw_addr);
            p->payload.dst_ip       = *((ipv4_addr_t *) &addr->addr);
            p->payload.dst_mac      = g_mac_broadcast;

            ret = iface->driver->tx(iface, (net_addr_t *) &g_mac_broadcast, np_arp, &buffer);
            buffer_deinit(&buffer);

            return ret;
        }
        else
            return EPROTONOSUPPORT;
    }
    else
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
    /* TODO */
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
*/
s32 arp_cache_add(const net_iface_t * const iface, const net_address_t *hw_addr,
                  const net_address_t *proto_addr)
{
    /* TODO - obtain lock on ARP cache during cache-add operation */
    arp_cache_item_t *p = arp_cache_get_entry_for_insert();
    if(p == NULL)
        return SUCCESS;     /* Cache disabled; fail silently */

    p->iface        = iface;
    p->etime        = g_current_timestamp + ARP_CACHE_ITEM_LIFETIME;
    p->hw_addr      = *hw_addr;
    p->proto_addr   = *proto_addr;

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
