/*
    Hardware device enumeration for the "lambda" (MC68010) motherboard

    Stuart Wallace, September 2015
*/

#include <board/lambda/device.h>
#include <board/lambda/mc68681.h>   /* DUART            */
#include <board/lambda/ds17485.h>   /* RTC              */
#include <board/lambda/ata.h>       /* ATA interface    */
#include <board/lambda/exp16.h>     /* Expansion slots  */

s32 b_dev_enumerate(dev_t *first_dev)
{
    /*
        Enumerate on-board devices (address range 0xe000000-0xefffff)
    */

    /* MC68681 DUART */
    if(mc68681_init() == SUCCESS)
    {

    }

    /* ATA interface */

    /* DS17485 RTC */

    /* Memory device */

    /*
        Enumerate expansion cards (if any)
    */
}
