/*
    Ethernet implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/ethernet.h>
#include <kernel/net/ipv4.h>
#include <kernel/net/arp.h>
#include <klibc/stdio.h>            /* FIXME remove */


mac_addr_t g_mac_broadcast =
{
    .b[0] = 0xff,
    .b[1] = 0xff,
    .b[2] = 0xff,
    .b[3] = 0xff,
    .b[4] = 0xff,
    .b[5] = 0xff
};


/* TODO: eth_print_mac() is special-case/useless.  do this differently */
void eth_print_mac(const mac_addr_t * const mac)
{
    printf("%02x:%02x:%02x:%02x:%02x:%02x",
           mac->b[0], mac->b[1], mac->b[2], mac->b[3], mac->b[4], mac->b[5]);
}


/*
    eth_handle_packet() - handle a received Ethernet packet
*/
s32 eth_handle_packet(net_iface_t *iface, const void *packet, u32 len)
{
    const eth_hdr_t * const ehdr = (eth_hdr_t *) packet;
    const void * const payload = ((u8 *) packet) + sizeof(eth_hdr_t);

    len -= sizeof(eth_hdr_t);

    switch(ehdr->type)
    {
        case ethertype_ipv4:
            return ipv4_handle_packet(iface, payload, len);

        case ethertype_arp:
            return arp_handle_packet(iface, payload, len);

        default:
            return SUCCESS;
    }
}


/*
    eth_transmit() - add an Ethernet header to the supplied frame and transmit it on the specified
    interface.
*/
s32 eth_transmit(net_iface_t *iface, const mac_addr_t *dest, const ethertype_t et, void *packet,
                 u32 len)
{
    void *buf = CHECKED_KMALLOC(sizeof(eth_hdr_t) + len);
    eth_hdr_t * const hdr = (eth_hdr_t *) buf;
    s32 ret;

    memcpy(&hdr->dest, dest, sizeof(mac_addr_t));
    memcpy(&hdr->src, &iface->hw_addr, sizeof(mac_addr_t));

    hdr->type = et;

    memcpy((void *) (hdr + 1), packet, len);

    ret = net_transmit(iface, buf, sizeof(eth_hdr_t) + len);

    kfree(buf);

    return ret;
}
