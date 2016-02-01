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


/*
    eth_tx() - add an Ethernet header to the supplied frame and transmit it on the specified
    interface.
*/
s32 eth_tx(net_iface_t *iface, net_addr_t *dest, ku16 type, buffer_t *payload)
{
    net_packet_t *p;
    eth_hdr_t *hdr;
    s32 ret;

    ret = net_alloc_packet(sizeof(eth_hdr_t) + payload->len, &p);
    if(ret != SUCCESS)
        return ret;

    hdr = buffer_dptr(p->raw);
    hdr->dest = *((mac_addr_t *) dest);
    hdr->src = *((mac_addr_t *) &iface->hw_addr.addr);
    hdr->type = type;
    p->payload = &hdr[1];
    p->payload_len = payload->len;

    memcpy(p->payload, buffer_dptr(payload), payload->len);

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
    eth_hdr_t * const ehdr = (eth_hdr_t *) buffer_dptr(packet->raw);

    ehdr->dest = ehdr->src;
    ehdr->src = *((mac_addr_t *) &packet->iface->hw_addr.addr);

    return net_transmit(packet);
}
