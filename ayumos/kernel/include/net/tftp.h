#ifndef KERNEL_INCLUDE_NET_TFTP_H_INC
#define KERNEL_INCLUDE_NET_TFTP_H_INC
/*
    TFTP implementation

    Part of ayumos


    (c) Stuart Wallace, May 2016.
*/

#ifdef WITH_NETWORKING

#include <kernel/include/defs.h>
#include <kernel/include/types.h>
#include <kernel/include/net/udp.h>
#include <kernel/include/net/net.h>


#define TFTP_SERVER_PORT        (69)

#define TFTP_OPCODE_LEN         (2)
#define TFTP_MAX_FN_LEN         (255)


/* TFTP op-codes */
typedef enum tftp_opcode
{
    tftp_rrq    = 1,
    tftp_wrq    = 2,
    tftp_data   = 3,
    tftp_ack    = 4,
    tftp_error  = 5
} tftp_opcode_t;


s32 tftp_read_request(net_address_t *peer, const char *fn);

#endif /* WITH_NETWORKING */
#endif
