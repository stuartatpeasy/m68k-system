/*
    ARP implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/arp.h>


/*
    arp_process_reply() - process an ARP reply packet.  Return EINVAL if the reply packet is
    invalid; returns ESUCCESS if the packet was successfully processed, or if the packet was
    ignored.  Currently only supports Ethernet+IP responses.
*/
s32 arp_process_reply(const void * const packet, u32 len)
{
    u8 * const pkt = (u8 *) packet;
    const arp_hdr_t * const hdr = (arp_hdr_t *) packet;
    const mac_addr_t *src_mac;
    const u32 *src_ip;

    /* Ensure that a complete header is present, and then verify that the packet is complete */
    if(len < sizeof(arp_hdr_t) || len < (2 * (hdr->hw_addr_len + hdr->proto_addr_len)))
        return EINVAL;

    /* Only interested in ARP responses */
    if((hdr->opcode         != BE2N16(arp_reply))
       || (hdr->hw_type     != BE2N16(arp_hw_type_ethernet))
       || (hdr->proto_type) != BE2N16(ethertype_ipv4))
        return SUCCESS;    /* Discard */

    src_mac = (mac_addr_t *) pkt + sizeof(arp_hdr_t);
    src_ip  = (u32 *) pkt + sizeof(arp_hdr_t) + sizeof(mac_addr_t);

    arp_cache_add(src_mac, src_ip);

    return SUCCESS;
}


/*
    arp_cache_add() - add a MAC address / IPv4 address pair to the ARP cache
*/
s32 arp_cache_add(const mac_addr_t *hw_addr, const u32 *ip)
{
    UNUSED(hw_addr);
    UNUSED(ip);

    /* TODO */

    return SUCCESS;
}
