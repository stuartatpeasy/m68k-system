/*
    Expansion card slot management

    Part of the as-yet-unnamed MC68010 operating system


    (c) Stuart Wallace, July 2015
*/

#include "expansion.h"


/*
    expansion_init() - initialise expansion card slots
*/
void expansion_init()
{
    u32 i;

	puts("Scanning expansion slots");

	for(i = 0; i < 4; ++i)
    {
        printf("slot %d: ", i);
        if(EXP_PRESENT(i))
        {
            /* A card is present; read its identity from the first byte of its address space */
            u8 id;

            EXP_ID_ASSERT();        /* Assert EID line to ask peripherals to identify themselves */
            id = *((u8 *) EXP_BASE(i));
            EXP_ID_NEGATE();

            switch(id)
            {
                default:
                    puts("unknown peripheral");
            }
        }
        else
            puts("vacant");
    }
}
