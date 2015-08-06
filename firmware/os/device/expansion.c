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
    u8 exp_pd;

	puts("Scanning expansion slots");
	exp_pd = read_expansion_card_presence_detect();

	for(i = 0; i < 4; ++i)
    {
        printf("slot %d: ", i);
        if(exp_pd & EXP_PD_MASK(i))
            puts("vacant");
        else
            puts("unknown peripheral");
    }
}
