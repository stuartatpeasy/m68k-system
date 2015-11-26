/*
	PS2Controller.c - PS/2 keyboard & mouse controller implementation
	
	This module contains firmware for an m68k-bus peripheral controller.
	
	
	(c) Stuart Wallace <stuartw@atom.net>, November 2015.
 */


#include <avr/io.h>
#include <avr/interrupt.h>

#include "macros.h"
#include "portdefs.h"
#include "registers.h"


#define PERIPHERAL_ID		(0x82)		/* Peripheral identifier */

unsigned char g_registers[16];

typedef enum state
{
	state_idle		= 0,
	state_start		= 1,
	state_d0		= 2,
	state_d1		= 3,
	state_d2		= 4,
	state_d3		= 5,
	state_d4		= 6,
	state_d5		= 7,
	state_d6		= 8,
	state_d7		= 9,
	state_parity	= 10,
	state_stop		= 11,
} state_t;

/*
	init() - initialise the chip following a reset
*/
void init(void)
{
	unsigned char i;
	
	DDRD			= PORTD_OUTPUTS;
	PORTD			= PORTD_PULLUPS;

	DDRC			= PORTC_OUTPUTS;
	PORTC			= PORTC_PULLUPS;

	DATA_BUS_DDR	= DATA_BUS_OUTPUTS;
	DATA_BUS		= DATA_BUS_PULLUPS;
	
	for(i = 0; i < sizeof(g_registers) / sizeof(g_registers[0]); ++i)
		g_registers[i] = 0;
}


/*
	set_register() - set a register to a new value, and act on the value if necessary
*/
void set_register(const unsigned char reg, const unsigned char data)
{
	g_registers[reg] = data;
	/* TODO: act on changes to registers as appropriate */
}


/*
	ISR for INT0 - handles communication with the host processor.
	This function is called when the host processor asserts nCS.
*/
ISR(INT0_vect)
{
	if(PORTD & nID)
	{
		/*
			This is an ID cycle.  Place the part identity on the data bus, assert nACK, and
			wait for the cycle to terminate.
		*/
		DO_READ_CYCLE(PERIPHERAL_ID);
	}
	else
	{
		/*
			This is a register read/write cycle.
		*/
		const unsigned char addr = PORTC & 0xF;

		if(PORTC & nUR)			/* Read cycle */
		{
			DO_READ_CYCLE(g_registers[addr]);
		}
		else if(PORTC & nUW)	/* Write cycle */
		{
			set_register(addr, DATA_BUS_PIN);
			TERMINATE_BUS_CYCLE();
		}
	}
}

/*
	main() - main loop: manage communications with the PS/2 devices
*/
int main(void)
{
	register unsigned char
		kbclk,
		kbclk_last = 0,
		mouseclk,
		mouseclk_last = 0,
		kbstate = state_idle,
		kbdata = 0,
		mousestate = state_idle,
		mousedata = 0;
	
	init();
	
	// TODO - ensure that PUD is 0 in SFIOR
	// TODO - configure interrupts
    while(1)
    {
		/* Read keyboard port */
		kbclk = PORTD & KB_CLK;
		if(kbclk != kbclk_last)
		{
			kbclk_last = kbclk;
			if(!kbclk)
			{
				/* Positive-to-negative edge on KB_CLK */
				switch(++kbstate)
				{
					case state_d0:
					case state_d1:
					case state_d2:
					case state_d3:
					case state_d4:
					case state_d5:
					case state_d6:
					case state_d7:
						kbdata >>= 1;
						if(PORTD & KB_DATA)
							kbdata |= 0x80;
						break;
					
					case state_parity:
						/* TODO handle parity */
						break;
						
					case state_stop:
						/* TODO: process received character */
						cli();
						g_registers[REG_KB_DATA] = kbdata;
						sei();
						kbstate = state_idle;
						break;
				}
			}
		}
		
		mouseclk = PORTD & MOUSE_CLK;
		if(mouseclk != mouseclk_last)
		{
			mouseclk_last = mouseclk;
			if(!mouseclk)
			{
				/* Positive-to-negative edge on MOUSE_CLK */
				switch(++mousestate)
				{
					case state_d0:
					case state_d1:
					case state_d2:
					case state_d3:
					case state_d4:
					case state_d5:
					case state_d6:
					case state_d7:
						mousedata >>= 1;
						if(PORTD & MOUSE_DATA)
							mousedata |= 0x80;
						break;
					
					case state_parity:
						/* TODO handle parity */
						break;
					
					case state_stop:
						/* TODO: process received character */
						cli();
						g_registers[REG_MOUSE_DATA] = mousedata;
						sei();
						mousestate = state_idle;
						break;
				}
			}
		}
    }
}
