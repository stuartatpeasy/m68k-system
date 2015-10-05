/*
    Hardware device enumeration for the "lambda" (MC68010) motherboard

    Stuart Wallace, September 2015
*/

#include <platform/platform.h>

#include <platform/lambda_rev0/device.h>
#include <platform/lambda_rev0/mc68681.h>   /* DUART            */
#include <platform/lambda_rev0/ds17485.h>   /* RTC              */
#include <platform/lambda_rev0/ata.h>       /* ATA interface    */
#include <platform/lambda_rev0/exp16.h>     /* Expansion slots  */

s32 b_dev_enumerate(dev_t *first_dev)
{
    /*
        Enumerate on-board devices (address range 0xe000000-0xefffff)
    */

    /* MC68681 DUART */
    dev_t duart;
    duart.base_addr = (void *) 0xe00000;
    mc68681_init(&duart);


    /* ATA interface */

    /* DS17485 RTC */

    /* Memory device */

    /*
        Enumerate expansion cards (if any)
    */

    return SUCCESS;
}
