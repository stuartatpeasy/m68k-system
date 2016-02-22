/*
    DHCP client implementation

    Part of ayumos


    (c) Stuart Wallace, February 2016
*/

#include <kernel/net/dhcp.h>
#include <kernel/net/udp.h>
#include <klibc/strings.h>


/*
    dhcp_rx() - handle an incoming DHCP packet.
*/
s32 dhcp_rx(net_packet_t *packet)
{
    dhcp_msg_t *msg = (dhcp_msg_t *) packet->start;

    if(packet->len < sizeof(dhcp_msg_t))
        return SUCCESS;     /* Drop truncated packet */

    UNUSED(msg);
    puts("dhcp_rx");

    return SUCCESS;
}


/*
    dhcp_discover() - send a DHCPDISCOVER packet on the specified interface
    Only Ethernet + IPv4 interfaces are supported.
*/
s32 dhcp_discover(net_iface_t *iface)
{
    dhcp_msg_t *msg;
    net_packet_t *pkt;
    net_address_t src, dest;
    u8 *opts;
    s32 ret;

    if((iface->hw_addr.type != na_ethernet) || (iface->proto_addr.type != na_ipv4))
        return EINVAL;

    ret = udp_alloc_packet(iface, DHCP_DISCOVER_BUFFER_LEN, &pkt);
    if(ret != SUCCESS)
        return ret;

    msg = (dhcp_msg_t *) pkt->start;
    opts = (u8 *) &(msg[1]);

    bzero(msg, DHCP_DISCOVER_BUFFER_LEN);

    msg->op             = bo_request;
    msg->htype          = 0x01;                     /* Hardware type = Ethernet */
    msg->hlen           = NET_ADDR_LEN_ETHERNET;
    msg->hops           = 0x00;
    msg->xid            = rand();
    msg->secs           = 0x0000;
    msg->flags          = 0x0000;
    msg->ciaddr         = (ipv4_addr_t) 0;
    msg->yiaddr         = (ipv4_addr_t) 0;
    msg->siaddr         = (ipv4_addr_t) 0;
    msg->giaddr         = (ipv4_addr_t) 0;
    msg->magic_cookie   = DHCP_MAGIC_COOKIE;

    memcpy(msg->chaddr, &iface->hw_addr.addr, NET_ADDR_LEN_ETHERNET);

    /* HACK - add options after msg */
    *opts++ = do_msg_type;
    *opts++ = 1;
    *opts++ = dmt_discover;

    *opts++ = do_end;       /* End of DHCP options */

    pkt->len = opts - (u8 *) pkt->start;

    return udp_tx(ipv4_make_addr(IPV4_ADDR_NONE, DHCP_CLIENT_PORT, &src),
                    ipv4_make_addr(IPV4_ADDR_BROADCAST, DHCP_SERVER_PORT, &dest), pkt);
}
