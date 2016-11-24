/*
    DHCP client implementation

    Part of ayumos


    (c) Stuart Wallace, February 2016

    FIXME: remove debug printf()s and remove #include stdio.h
*/

#include <kernel/net/dhcp.h>
#include <kernel/net/udp.h>
#include <kernel/net/packet.h>
#include <kernel/net/protocol.h>
#include <klibc/stdio.h>        // FIXME remove
#include <klibc/string.h>
#include <klibc/strings.h>


/*
    dhcp_rx() - handle an incoming DHCP packet.
*/
s32 dhcp_rx(net_packet_t *packet)
{
    net_iface_t *iface;
    dhcp_msg_t *msg = (dhcp_msg_t *) net_packet_get_start(packet);

    if(net_packet_get_len(packet) < sizeof(dhcp_msg_t))
        return SUCCESS;     /* Drop truncated packet */

    iface = net_packet_get_interface(packet);

    if((msg->op == bo_reply) && (msg->magic_cookie == DHCP_MAGIC_COOKIE))
    {
        if((msg->htype == bh_ethernet) && (msg->hlen == NET_ADDR_LEN_ETHERNET)
           && net_address_get_type(net_interface_get_hw_addr(iface)) == na_ethernet)
        {
            /* This is an BOOTP+Ethernet response, received on an Ethernet interface */
            const mac_addr_t * const bootp_hwaddr = (mac_addr_t *) msg->chaddr,
                             * const iface_hwaddr = eth_get_addr(net_interface_get_hw_addr(iface));

            if(MAC_ADDR_EQUAL(bootp_hwaddr, iface_hwaddr))
            {
                /* The message is addressed to us */
                /* FIXME - check that the transaction ID matches the one in the iface DHCP status */
                void *p, *end;
                const dhcp_option_t *opt;
//                dhcp_msg_type_t msg_type;     // FIXME
                char cbuf[16], ybuf[16], sbuf[16], gbuf[16];
                net_address_t c, y, s, g;

                c.type = y.type = s.type = g.type = na_ipv4;

                *((ipv4_addr_t *) &c.addr) = msg->ciaddr;
                *((ipv4_addr_t *) &y.addr) = msg->yiaddr;
                *((ipv4_addr_t *) &s.addr) = msg->siaddr;
                *((ipv4_addr_t *) &g.addr) = msg->giaddr;

                net_address_print(&c, cbuf, sizeof(cbuf));
                net_address_print(&y, ybuf, sizeof(ybuf));
                net_address_print(&s, sbuf, sizeof(sbuf));
                net_address_print(&g, gbuf, sizeof(gbuf));

                /* Process DHCP options */
                p = net_packet_get_start(packet) + sizeof(dhcp_msg_t);
                end = net_packet_get_start(packet) + net_packet_get_len(packet);

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
//                            msg_type = opt->data;     // FIXME
                            break;

                        case dopt_subnet_mask:
                            memcpy(&addr.addr, &opt->data, sizeof(ipv4_addr_t));
                            net_address_print(&addr, addrbuf, sizeof(addrbuf));
                            printf("Subnet mask: %s\n", addrbuf);
                            break;

                        case dopt_router:
                            memcpy(&addr.addr, &opt->data, sizeof(ipv4_addr_t));
                            net_address_print(&addr, addrbuf, sizeof(addrbuf));
                            printf("Router: %s\n", addrbuf);
                            break;

                        case dopt_domain_name_server:
                            memcpy(&addr.addr, &opt->data, sizeof(ipv4_addr_t));
                            net_address_print(&addr, addrbuf, sizeof(addrbuf));
                            printf("DNS resolver: %s\n", addrbuf);
                            break;

                        case dopt_broadcast_addr:
                            memcpy(&addr.addr, &opt->data, sizeof(ipv4_addr_t));
                            net_address_print(&addr, addrbuf, sizeof(addrbuf));
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

    ipv4_make_addr(IPV4_ADDR_NONE, DHCP_CLIENT_PORT, &src);
    ipv4_make_addr(IPV4_ADDR_BROADCAST, DHCP_SERVER_PORT, &dest);

    ret = net_protocol_packet_alloc(np_udp, &dest, DHCP_DISCOVER_BUFFER_LEN, iface, &pkt);
    if(ret != SUCCESS)
        return ret;

    msg = (dhcp_msg_t *) net_packet_get_start(pkt);
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

    memcpy(msg->chaddr, eth_get_addr(net_interface_get_hw_addr(iface)), NET_ADDR_LEN_ETHERNET);

    /* HACK - add options after msg */
    *opts++ = dopt_msg_type;
    *opts++ = 1;
    *opts++ = dmt_discover;

    *opts++ = dopt_end;     /* End of DHCP options */

    ret = net_packet_set_len(pkt, opts - (u8 *) msg);
    if(ret != SUCCESS)
        return ret;

    ret = net_protocol_tx(&src, &dest, pkt);

    net_packet_free(pkt);

    return ret;
}
