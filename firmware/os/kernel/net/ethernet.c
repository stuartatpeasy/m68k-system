/*
    Ethernet implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/ethernet.h>
#include <kernel/net/ipv4.h>
#include <kernel/net/arp.h>
#include <klibc/stdio.h>
#include <klibc/strings.h>


const net_address_t g_eth_broadcast =
{
    .type = na_ethernet,
    .addr.addr_bytes[0] = 0xff,
    .addr.addr_bytes[1] = 0xff,
    .addr.addr_bytes[2] = 0xff,
    .addr.addr_bytes[3] = 0xff,
    .addr.addr_bytes[4] = 0xff,
    .addr.addr_bytes[5] = 0xff
};


/*
    eth_init() - initialise Ethernet protocol driver
*/
s32 eth_init(net_proto_driver_t *driver)
{
    driver->name            = "Ethernet";
    driver->proto           = np_ethernet;
    driver->rx              = eth_rx;
    driver->tx              = eth_tx;

    return SUCCESS;
}


/*
    eth_rx() - handle an incoming Ethernet packet.
*/
s32 eth_rx(net_packet_t *packet)
{
    const eth_hdr_t * const ehdr = (eth_hdr_t *) packet->raw.data;

    packet->start += sizeof(eth_hdr_t);
    packet->len -= sizeof(eth_hdr_t);

    switch(ehdr->type)
    {
        case ethertype_ipv4:
            packet->proto = np_ipv4;
            return ipv4_rx(packet);
            break;

        case ethertype_arp:
            packet->proto = np_arp;
            return arp_rx(packet);
            break;
    }

    return EPROTONOSUPPORT;
}


/*
    eth_tx() - add an Ethernet header to the supplied frame and transmit it on the specified
    interface.
*/
s32 eth_tx(const net_address_t *src, const net_address_t *dest, net_packet_t *packet)
{
    eth_hdr_t *hdr;
    const mac_addr_t *src_addr, *dest_addr;

    packet->start -= sizeof(eth_hdr_t);
    packet->len += sizeof(eth_hdr_t);
    hdr = (eth_hdr_t *) packet->start;

    switch(packet->proto)
    {
        case np_ipv4:
            hdr->type = ethertype_ipv4;
            break;

        case np_arp:
            hdr->type = ethertype_arp;
            break;

        default:
            return EPROTONOSUPPORT;
    }

    /*
        If src is NULL, send from the default address of the interface; otherwise send from the
        specified address.
    */
    src_addr = (src == NULL) ? (mac_addr_t *) &packet->iface->hw_addr.addr
                                : (mac_addr_t *) &src->addr.addr;
    dest_addr = (mac_addr_t *) &dest->addr.addr;

    hdr->dest = *dest_addr;
    hdr->src = *src_addr;

    return net_tx(src, dest, packet);
}

#if 0   // FIXME
/*
    eth_reply() - assume that *packet contains a received packet which has been modified in some
    way; swap its source and destination addresses and transmit it.
*/
s32 eth_reply(net_packet_t *packet)
{
    eth_hdr_t *ehdr;
    net_address_t src, dest;

    packet->start -= sizeof(eth_hdr_t);
    packet->len += sizeof(eth_hdr_t);

    ehdr = (eth_hdr_t *) packet->start;

    eth_make_addr(&ehdr->dest, &src);
    eth_make_addr(NULL, &dest);

    ehdr->dest = *((mac_addr_t *) &dest.addr);
    ehdr->src = *((mac_addr_t *) &src.addr);

    return net_tx(&src, &dest, packet);
}
#endif


/*
    eth_make_addr() - populate a net_address_t object with a MAC address.
*/
void eth_make_addr(mac_addr_t *mac, net_address_t *addr)
{
    net_address_set_type(na_ethernet, addr);
    bzero(&addr->addr, sizeof(net_addr_t));

    if(mac != NULL)
        memcpy(&addr->addr, mac, sizeof(mac_addr_t));
}


/*
    eth_packet_alloc() - allocate a net_packet_t object large enough to contain an Ethernet header
    and a payload of the specified length.
*/
s32 eth_packet_alloc(net_address_t * addr, ku32 len, net_iface_t *iface, net_packet_t **packet)
{
    ks32 ret = net_packet_alloc(np_ethernet, addr, sizeof(eth_hdr_t) + len, iface, packet);

    if(ret != SUCCESS)
        return ret;

    net_packet_consume(sizeof(eth_hdr_t), *packet);

    return SUCCESS;
}


/*
    eth_print_addr() - write addr to buf
*/
s32 eth_print_addr(const mac_addr_t *addr, char *buf, s32 len)
{
    return snprintf(buf, len, "%02x:%02x:%02x:%02x:%02x:%02x",
                    addr->b[0], addr->b[1], addr->b[2], addr->b[3], addr->b[4], addr->b[5]);
}
