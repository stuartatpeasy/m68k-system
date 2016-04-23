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

    if((msg->op == bo_reply) && (msg->magic_cookie == DHCP_MAGIC_COOKIE))
    {
        if((msg->htype == bh_ethernet) && (msg->hlen == NET_ADDR_LEN_ETHERNET)
           && packet->iface->hw_addr.type == na_ethernet)
        {
            /* This is an BOOTP+Ethernet response, received on an Ethernet interface */
            const mac_addr_t * const bootp_hwaddr = (mac_addr_t *) msg->chaddr,
                             * const iface_hwaddr = (mac_addr_t *) &packet->iface->hw_addr.addr;

            if(MAC_ADDR_EQUAL(bootp_hwaddr, iface_hwaddr))
            {
                /* The message is addressed to us */
                /* FIXME - check that the transaction ID matches the one in the iface DHCP status */
                void *p, *end;
                const dhcp_option_t *opt;
                dhcp_msg_type_t msg_type;
                char cbuf[16], ybuf[16], sbuf[16], gbuf[16];
                net_address_t c, y, s, g;

                c.type = y.type = s.type = g.type = na_ipv4;

                *((ipv4_addr_t *) &c.addr) = msg->ciaddr;
                *((ipv4_addr_t *) &y.addr) = msg->yiaddr;
                *((ipv4_addr_t *) &s.addr) = msg->siaddr;
                *((ipv4_addr_t *) &g.addr) = msg->giaddr;

                net_print_addr(&c, cbuf, sizeof(cbuf));
                net_print_addr(&y, ybuf, sizeof(ybuf));
                net_print_addr(&s, sbuf, sizeof(sbuf));
                net_print_addr(&g, gbuf, sizeof(gbuf));

                /* Process DHCP options */
                p = packet->start + sizeof(dhcp_msg_t);
                end = packet->start + packet->len;

                for(; p < end; p += sizeof(dhcp_option_hdr_t) + opt->hdr.len)
                {
                    char addrbuf[16];
                    net_address_t addr;

                    addr.type = na_ipv4;

                    opt = (dhcp_option_t *) p;
                    switch(opt->hdr.id)
                    {
                        case dopt_padding:
                            ++p;
                            break;

                        case dopt_msg_type:
                            msg_type = opt->data;
                            break;

                        case dopt_subnet_mask:
                            memcpy(&addr.addr, &opt->data, sizeof(ipv4_addr_t));
                            net_print_addr(&addr, addrbuf, sizeof(addrbuf));
                            printf("Subnet mask: %s\n", addrbuf);
                            break;

                        case dopt_router:
                            memcpy(&addr.addr, &opt->data, sizeof(ipv4_addr_t));
                            net_print_addr(&addr, addrbuf, sizeof(addrbuf));
                            printf("Router: %s\n", addrbuf);
                            break;

                        case dopt_domain_name_server:
                            memcpy(&addr.addr, &opt->data, sizeof(ipv4_addr_t));
                            net_print_addr(&addr, addrbuf, sizeof(addrbuf));
                            printf("DNS resolver: %s\n", addrbuf);
                            break;

                        case dopt_broadcast_addr:
                            memcpy(&addr.addr, &opt->data, sizeof(ipv4_addr_t));
                            net_print_addr(&addr, addrbuf, sizeof(addrbuf));
                            printf("Broadcast: %s\n", addrbuf);
                            break;

                        default:
                            printf("id=%d  len=%d\n", opt->hdr.id, opt->hdr.len);
                            break;
                    }

                    if(opt->hdr.id == dopt_end)
                        break;
                }

                printf("BOOTP reply\n"
                       "CIaddr: %s\nYIaddr: %s\nSIaddr: %s\nGIaddr: %s\n",
                       cbuf, ybuf, sbuf, gbuf);
            }
        }
        /* Discard non-Ethernet-related packets */
    }

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

    /* FIXME - only set BOOTP_FLAG_BROADCAST in msg->flags if interface is not configured */

    msg->op             = bo_request;
    msg->htype          = bh_ethernet;
    msg->hlen           = NET_ADDR_LEN_ETHERNET;
    msg->hops           = 0x00;
    msg->xid            = rand32();
    msg->secs           = 0x0000;
    msg->flags          = BOOTP_FLAG_BROADCAST;
    msg->ciaddr         = (ipv4_addr_t) 0;
    msg->yiaddr         = (ipv4_addr_t) 0;
    msg->siaddr         = (ipv4_addr_t) 0;
    msg->giaddr         = (ipv4_addr_t) 0;
    msg->magic_cookie   = DHCP_MAGIC_COOKIE;

    memcpy(msg->chaddr, &iface->hw_addr.addr, NET_ADDR_LEN_ETHERNET);

    /* HACK - add options after msg */
    *opts++ = dopt_msg_type;
    *opts++ = 1;
    *opts++ = dmt_discover;

    *opts++ = dopt_end;     /* End of DHCP options */

    pkt->len = opts - (u8 *) pkt->start;

    return udp_tx(ipv4_make_addr(IPV4_ADDR_NONE, DHCP_CLIENT_PORT, &src),
                    ipv4_make_addr(IPV4_ADDR_BROADCAST, DHCP_SERVER_PORT, &dest), pkt);
}
