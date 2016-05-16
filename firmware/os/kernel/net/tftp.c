/*
    TFTP implementation

    Part of ayumos


    (c) Stuart Wallace, May 2016.
*/

#include <kernel/net/tftp.h>
#include <kernel/net/ipv4.h>
#include <kernel/net/ipv4route.h>       // FIXME - remove IPv4-specific #include [?]
#include <kernel/net/packet.h>
#include <klibc/string.h>


/*
    tftp_read_request: send a request for file fn to the specified peer.
*/
s32 tftp_read_request(net_address_t *peer_addr, const char *fn)
{
    net_address_t src;
    net_packet_t *pkt;
    net_iface_t *iface;
    ipv4_address_t *ipv4_addr;
    s32 ret;
    u32 fn_len;
    char *p;

    /* TODO: validate filename */
    fn_len = strlen(fn);
    if(!fn_len || (fn_len > TFTP_MAX_FN_LEN))
        return EINVAL;

    if(peer_addr->type != na_ipv4)
        return EPROTONOSUPPORT;     /* Only IPv4 addresses are supported at the moment */

    ret = ipv4_route_get_iface(peer_addr, &iface);
    if(ret != SUCCESS)
        return ret;

    /* Force the server port to the TFTP well-known port number */
    ipv4_addr = (ipv4_address_t *) &peer_addr->addr;
    ipv4_addr->port = TFTP_SERVER_PORT;


    /*
        The read request packet length is equal to the size of the read request opcode (2 bytes)
        plus the length (including terminating '\0) of the requested filename, plus the length
        (including terminating '\0') of the transfer mode.  The transfer mode is always "octet".
    */
    ret = udp_packet_alloc(peer_addr, TFTP_OPCODE_LEN + fn_len + 7, NET_INTERFACE_ANY, &pkt);
    if(ret != SUCCESS)
        return ret;

    *((u16 *) net_packet_get_start(pkt)) = N2BE16(tftp_rrq);    /* Opcode: read request */
    p = (char *) net_packet_get_start(pkt) + TFTP_OPCODE_LEN;

    strcpy(p, fn);
    strcpy(p + fn_len + 1, "octet");

    /* FIXME - get ephemeral port num, instead of using 12345 */
    ret = udp_tx(ipv4_make_addr(IPV4_ADDR_NONE, 12345, &src), peer_addr, pkt);
    net_packet_free(pkt);

    return ret;
}
