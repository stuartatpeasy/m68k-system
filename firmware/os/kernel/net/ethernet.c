/*
    Ethernet implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/ethernet.h>
#include <kernel/net/ipv4.h>
#include <kernel/net/arp.h>


const mac_addr_t g_mac_broadcast =
{
    .b[0] = 0xff,
    .b[1] = 0xff,
    .b[2] = 0xff,
    .b[3] = 0xff,
    .b[4] = 0xff,
    .b[5] = 0xff
};


/*
    eth_identify_proto() - identify the protocol of data within an Ethernet packet.
*/
s32 eth_identify_proto(net_packet_t *packet)
{
    const eth_hdr_t * const ehdr = (eth_hdr_t *) buffer_dptr(packet->raw);
    net_protocol_t proto;

    switch(ehdr->type)
    {
        case ethertype_ipv4:
            proto = np_ipv4;
            break;

        case ethertype_arp:
            proto = np_arp;
            break;

        default:
            return EPROTONOSUPPORT;
    }

    packet->proto = proto;
    packet->payload = (void *) &ehdr[1];
    packet->payload_len = packet->raw->len - sizeof(eth_hdr_t);

    return SUCCESS;
}


s32 eth_tx(net_packet_t *packet)
{
    return net_transmit(packet->iface, buffer_dptr(packet->raw), packet->raw->len);
}


s32 eth_reply(net_packet_t *packet)
{
    eth_hdr_t * const ehdr = (eth_hdr_t *) buffer_dptr(packet->raw);

    ehdr->dest = ehdr->src;
    ehdr->src = *((mac_addr_t *) &packet->iface->hw_addr.addr);

    return eth_tx(packet);;
}


/*
    eth_transmit() - add an Ethernet header to the supplied frame and transmit it on the specified
    interface.
*/
s32 eth_transmit(net_iface_t *iface, const mac_addr_t *dest, const ethertype_t et,
                 buffer_t *packet)
{
    void *buf = CHECKED_KMALLOC(sizeof(eth_hdr_t) + packet->len);
    eth_hdr_t * const hdr = (eth_hdr_t *) buf;
    s32 ret;

    /* TODO - rewrite this a bit; make fuller use of buffer_t */
    hdr->dest = *dest;
    hdr->src = *((mac_addr_t *) &iface->hw_addr.addr);
    hdr->type = et;

    memcpy((void *) (hdr + 1), buffer_dptr(packet), packet->len);

    ret = net_transmit(iface, buf, sizeof(eth_hdr_t) + packet->len);

    kfree(buf);

    return ret;
}
