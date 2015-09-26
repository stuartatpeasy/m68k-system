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
    expansion_root_t root;

	puts("Scanning expansion slots");

	for(root.base = EXP_BASE_ADDR, root.len = EXP_ADDR_LEN, root.irql = EXP_BASE_IRQ, i = 0;
        i < EXP_NUM_SLOTS;
        ++i, root.base += EXP_ADDR_LEN, ++root.irql)
    {
        printf("slot %d (0x%08x-0x%08x irq %u): ", i, root.base, root.base + root.len - 1,
               root.irql);

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
                    printf("unknown peripheral %02x\n", id);
            }
        }
        else
            puts("vacant");
    }
}
