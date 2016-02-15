/*
    DHCP client implementation

    Part of ayumos


    (c) Stuart Wallace, February 2016
*/

#include <kernel/net/dhcp.h>
#include <klibc/strings.h>

/*
    dhcp_discover() - send a DHCPDISCOVER packet on the specified interface
    Only Ethernet + IPv4 interfaces are supported.
*/
s32 dhcp_discover(net_iface_t *iface)
{
    u8 *buf, *opts;
    dhcpdiscover_msg_t *msg;

    if((iface->hw_addr.type != na_ethernet) || (iface->proto_addr.type != na_ipv4))
        return EINVAL;

    buf = kcalloc(1, DHCP_DISCOVER_BUFFER_LEN);
    if(buf == NULL)
        return ENOMEM;

    msg = (dhcpdiscover_msg_t *) buf;
    opts = buf + sizeof(dhcpdiscover_msg_t);

    msg->op             = bo_request;
    msg->htype          = 0x01;                     /* Hardware type = Ethernet */
    msg->hlen           = NET_ADDR_LEN_ETHERNET;
/*  msg->hops           = 0x00;                 */
    msg->xid            = rand();
/*  msg->secs           = 0x0000;               */
/*  msg->flags          = 0x0000;               */
/*  msg->ciaddr         = (ipv4_addr_t) 0;      */
/*  msg->yiaddr         = (ipv4_addr_t) 0;      */
/*  msg->siaddr         = (ipv4_addr_t) 0;      */
/*  msg->giaddr         = (ipv4_addr_t) 0;      */
    msg->magic_cookie   = DHCP_MAGIC_COOKIE;

    memcpy(msg->chaddr, &iface->hw_addr.addr, NET_ADDR_LEN_ETHERNET);

    /* HACK - add options after msg */
    *opts++ = do_msg_type;
    *opts++ = 1;
    *opts++ = dmt_discover;

    *opts++ = do_end;       /* End of DHCP options */

//    udp_send(IPV4_ADDR_NONE, IPV4_ADDR_BROADCAST, )

    kfree(buf);
    return SUCCESS;
}
