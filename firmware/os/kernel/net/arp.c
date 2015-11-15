/*
    ARP implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/arp.h>
#include <kernel/memory/kmalloc.h>
#include <stdio.h>              /* FIXME: remove */


/*
    arp_handle_packet() - handle an incoming ARP packet.  Return EINVAL if the packet is invalid;
    returns ESUCCESS if the packet was successfully processed, or if the packet was ignored.
    Currently only supports Ethernet+IP responses.
*/
s32 arp_handle_packet(net_iface_t *iface, const void * const packet, u32 len)
{
    const arp_hdr_t * const hdr = (arp_hdr_t *) packet;
    const arp_payload_t * const payload = (arp_payload_t *) ((u8 *) packet + sizeof(arp_hdr_t));

    printf("  ARP: len=%d\n", len);

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
        /*
            TODO:
                - send ARP response
                - maybe update ARP cache with senders hw/proto address?
        */
        arp_eth_ipv4_packet_t p;

        arp_cache_add(payload->src_mac, payload->src_ip);

        p.hdr.hw_type           = arp_hw_type_ethernet;
        p.hdr.proto_type        = ethertype_ipv4;
        p.hdr.hw_addr_len       = sizeof(mac_addr_t);
        p.hdr.proto_addr_len    = sizeof(ipv4_addr_t);
        p.hdr.opcode            = arp_reply;

        p.payload.dst_ip        = payload->src_ip;
        p.payload.dst_mac       = payload->dst_mac;
        p.payload.src_ip        = *((ipv4_addr_t *) &iface->proto_addr);
        p.payload.src_mac       = *((mac_addr_t *) &iface->hw_addr);

//      return iface->send_packet(p, sizeof(p));
        UNUSED(p);
    }
    else if(hdr->opcode == BE2N16(arp_reply))
    {
        /* This is an Ethernet+IPv4 ARP reply.  Add the data to the cache. */
        return arp_cache_add(payload->src_mac, payload->src_ip);
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

    p.hdr.hw_type           = arp_hw_type_ethernet;
    p.hdr.proto_type        = ethertype_ipv4;
    p.hdr.hw_addr_len       = sizeof(mac_addr_t);
    p.hdr.proto_addr_len    = sizeof(ipv4_addr_t);
    p.hdr.opcode            = arp_request;

    p.payload.src_ip        = *((ipv4_addr_t *) &iface->proto_addr);
    p.payload.src_mac       = *((mac_addr_t *) &iface->hw_addr);
    p.payload.dst_ip        = ip;
    p.payload.dst_mac       = g_mac_zero;

//  return iface->send_packet(p, sizeof(p));
    UNUSED(p);

    return SUCCESS;
}


/*
    arp_cache_add() - add a MAC address / IPv4 address pair to the ARP cache
*/
s32 arp_cache_add(const mac_addr_t hw_addr, const ipv4_addr_t ip)
{
    UNUSED(hw_addr);
    UNUSED(ip);

    /* TODO */

    return SUCCESS;
}
