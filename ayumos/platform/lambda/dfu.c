/*
    Device firmware update code

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, January 2012.


    This module manages the process of re-flashing system ROMs.

    Note: The reprogramming algorithm is designed to suit AMIC A29040B/C 512KBx8 Flash ROMs.
    It may not be suitable for other devices.
*/

#include <kernel/include/platform.h>
#include <platform/lambda/include/dfu.h>


void write_flash(ku16 *data, ku32 len)
{
    u32 x;

    /*
        Erase sectors as required.  Sectors are 64K each per chip, i.e. 64Kx16.
    */
    int sectors_to_erase = len >> 17;

    do
    {
        /* Issue a sector-erase command */
        FLASH_REG1 = 0xaaaa;
        FLASH_REG2 = 0x5555;
        FLASH_REG1 = 0x8080;
        FLASH_REG1 = 0xaaaa;
        FLASH_REG2 = 0x5555;

        FLASH_OFFSET(sectors_to_erase << 17) = 0x3030;

        /*
            Wait for the sector erase operation to complete.

            While the erase operation is in progress, each device will output 0 on its D7 pin.
            When the operation completes, the device will output a 1 on D7.
        */
        while((FLASH_OFFSET(0) & 0x8080) != 0x8080) ;
    } while(sectors_to_erase--);

    for(x = 0; x < (len >> 1); x++)
    {
        FLASH_REG1 = 0xaaaa;
        FLASH_REG2 = 0x5555;
        FLASH_REG1 = 0xa0a0;

        FLASH_OFFSET(x) = data[x];

        /*
            Wait for program operation to complete.

            While the program operation is in progress, each device will output the complement of
            the the programmed MSB on its D7 pin.  Once each byte has been programmed, the value
            of D7 will be equal to the programmed MSB.
        */
        while((FLASH_OFFSET(x) & 0x8080) != (data[x] & 0x8080)) ;
    }

    /* Reset the system */
    PLAT_DO_RESET;              /* Won't return */
}

/*
    dfu() - initiate an in-system firmware update.

    Note: len must be even.  If the firmware size is not even, a padding byte (probably a 0x00) must
    be added to the end of *data, and len must be incremented, before this function is called.
*/
s32 dfu(ku16 *data, ku32 len)
{
    /*
        On entry, data will point to the new firmware (in RAM); len will specify the number of
        data bytes to be written.
    */
    void (*p_write_flash)(ku16 *, ku32);
    ku32 write_flash_len = (ku32) dfu - (ku32) write_flash;

    if(!len || (len > LAMBDA_ROM_LENGTH) || (len & 1))
        return -EINVAL;

    p_write_flash = (void (*)(ku16 *, ku32)) kmalloc(write_flash_len);
    if(p_write_flash == NULL)
        return -ENOMEM;

    /* Copy the Flash-update routine into RAM */
    memcpy(p_write_flash, write_flash, write_flash_len);

    cpu_disable_interrupts();

    /* This shouldn't return */
    p_write_flash(data, len);

    return -EUNKNOWN;   /* Should not be reached */
}

