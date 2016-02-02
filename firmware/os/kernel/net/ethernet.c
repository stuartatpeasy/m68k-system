/*
    Ethernet implementation

    Part of ayumos


    (c) Stuart Wallace, November 2015.
*/

#include <kernel/net/ethernet.h>
#include <kernel/net/ipv4.h>
#include <kernel/net/arp.h>
#include <klibc/strings.h>


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
    eth_init() - initialise Ethernet protocol driver
*/
s32 eth_init(net_proto_driver_t *driver)
{
    driver->name = "Ethernet";
    driver->proto = np_ethernet;
//    driver->rx = eth_rx;          // FIXME
    driver->tx = eth_tx;
    driver->reply = eth_reply;

    return SUCCESS;
}

/*
    eth_rx() - identify the protocol of data within an Ethernet packet.
*/
s32 eth_rx(net_packet_t *packet)
{
    const eth_hdr_t * const ehdr = (eth_hdr_t *) packet->raw.data;
    net_packet_t inner;

    switch(ehdr->type)
    {
        case ethertype_ipv4:
            inner.proto = np_ipv4;
            break;

        case ethertype_arp:
            inner.proto = np_arp;
            break;

        default:
            return EPROTONOSUPPORT;
    }

    inner.iface = packet->iface;
    inner.raw.len = packet->raw.len - sizeof(eth_hdr_t);
    inner.raw.data = (void *) &ehdr[1];
    inner.driver = net_get_proto_driver(inner.proto);////// FIXME - should be eth_get_proto_driver() ??? maybe not
    inner.parent = packet;

    return inner.driver->rx(&inner);
}


/*
    eth_tx() - add an Ethernet header to the supplied frame and transmit it on the specified
    interface.
*/
s32 eth_tx(net_iface_t *iface, net_addr_t *dest, ku16 proto, buffer_t *payload)
{
    net_packet_t *p;
    eth_hdr_t *hdr;
    s32 ret;

    ret = net_alloc_packet(sizeof(eth_hdr_t) + payload->len, &p);
    if(ret != SUCCESS)
        return ret;

    hdr = p->raw.data;

    switch(proto)
    {
        case np_ipv4:
            hdr->type = ethertype_ipv4;
            break;

        case np_arp:
            hdr->type = ethertype_arp;
            break;

        default:
            net_free_packet(p);
            return EPROTONOSUPPORT;
    }

    hdr->dest = *((mac_addr_t *) dest);
    hdr->src = *((mac_addr_t *) &iface->hw_addr.addr);
    p->raw.data = &hdr[1];
    p->raw.len = payload->len;

    memcpy(p->raw.data, payload->data, payload->len);

    ret = net_transmit(p);

    net_free_packet(p);

    return ret;
}


/*
    eth_reply() - assume that *packet contains a received packet which has been modified in some
    way; swap its source and destination addresses and transmit it.
*/
s32 eth_reply(net_packet_t *packet)
{
    eth_hdr_t * const ehdr = (eth_hdr_t *) packet->raw.data;

    ehdr->dest = ehdr->src;
    ehdr->src = *((mac_addr_t *) &packet->iface->hw_addr.addr);

    return net_transmit(packet);
}


/*
    eth_make_addr() - populate a net_address_t object with a MAC address.
*/
void eth_make_addr(mac_addr_t *mac, net_address_t *addr)
{
    addr->type = na_ethernet;
    bzero(&addr->addr, sizeof(net_addr_t));
    memcpy(&addr->addr, mac, sizeof(mac_addr_t));
}
