/*
	PS2Controller.c - PS/2 keyboard & mouse controller implementation
	
	This module contains firmware for an m68k-bus peripheral controller.
	
	
	(c) Stuart Wallace <stuartw@atom.net>, November 2015.
 */

#define F_CPU		8000000UL		/* 8MHz CPU clock */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define WITH_DEBUGGING

#include "debug.h"
#include "macros.h"
#include "portdefs.h"
#include "registers.h"

#define PERIPHERAL_ID		(0x82)		/* Peripheral identifier */

unsigned char check_odd_parity(unsigned char x, const unsigned char parity);

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


unsigned char	g_registers[16],
				kbstate = state_idle,
				mousestate = state_idle,
				kbdata,
				kbparity,
				mousedata,
				mouseparity;


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

	/* Enable INT0 and INT1 interrupts on falling edge */
	MCUCR = (MCUCR & ~(_BV(ISC11) | _BV(ISC10) | _BV(ISC01) | _BV(ISC00)))
			| _BV(ISC11) | _BV(ISC01);
	
	GICR |= _BV(INT0) | _BV(INT1);
	
	debug_init();
	
	sei();
}


/*
	host_reg_write() - handle a register write by the host.
*/
void host_reg_write(const unsigned char reg, const unsigned char data)
{
	cli();
	g_registers[reg] = data;
	sei();
	/* TODO: act on changes to registers as appropriate */
}


void kb_send_command(unsigned char cmd)
{
	unsigned char i;
	
	// TODO: generate parity of cmd
	
	/* Wait for any in-progress receive to complete */
	while(kbstate != state_idle)
		;
	
	cli();					/* Ignore any received data while the command is sent */
	
	DDRD |= KB_CLK;			/* Set keyboard clock line to output		*/
	PORTD &= ~KB_CLK;		/* Pull clock line low						*/
	
	_delay_us(60);			/* Wait 60us to get the device's attention	*/

	DDRD |= KB_DATA;		/* Set keyboard data line to output			*/
	PORTD &= ~KB_DATA;		/* Pull data line low						*/

	DDRD &= ~KB_CLK;		/* Release clock line (set it to input)		*/
	
	while(PIND & KB_CLK)	/* Wait for device to pull clock line low	*/
		;

	for(i = 8; i; --i, cmd >>= 1)
	{
		/* Wait for device to pull clock line high */
		while(!(PIND & KB_CLK))
			;
			
		/* Send the next bit of the command */
		if(cmd & 1)
			PORTD |= KB_DATA;
		else
			PORTD &= ~KB_DATA;
		
		/* Wait for device to pull clock line low */
		while(PIND & KB_CLK)
			;
	}
	
	/* Wait for device to pull clock line high */
	while(!(PIND & KB_CLK))
		;
	
	// TODO: send parity
	
	/* Wait for device to pull clock line low */
	while(PIND & KB_CLK)
		;

	DDRD &= ~KB_CLK;		/* Set data line to input					*/
	
	sei();					/* Re-enable interrupts						*/
}


/*
	ISR for INT0 - handles keyboard clock transitions.
*/
ISR(INT0_vect)
{
	/* Read keyboard port */
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
			if(PIND & KB_DATA)
				kbdata |= 0x80;
			break;
			
		case state_parity:
			kbparity = (PIND & KB_DATA) ? 1 : 0;
			break;
			
		case state_stop:
			if(check_odd_parity(kbdata, kbparity))
			{
				debug_puthexb(kbdata);			/* TODO: process received data */
				g_registers[REG_KB_DATA] = kbdata;
			}
			else
			{
				/* Parity error - drop data */
				debug_putc('!');
				debug_puthexb(kbdata);
			}
			debug_putc('\n');

			kbstate = state_idle;
			break;
	}
}


/*
	ISR for INT1 - handles mouse clock transitions.
*/
ISR(INT1_vect)
{
	/* Read mouse port */
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
			if(PIND & MOUSE_DATA)
				mousedata |= 0x80;
			break;
					
		case state_parity:
			mouseparity = (PIND & MOUSE_DATA) ? 1 : 0;
			break;
					
		case state_stop:
			if(check_odd_parity(mousedata, mouseparity))
			{
				debug_puthexb(mousedata);			/* TODO: process received data */
				debug_putc('\n');
				g_registers[REG_MOUSE_DATA] = mousedata;
			}
			else
			{
				/* Parity error - drop data */
				debug_putc('!');
			}
			
			mousestate = state_idle;
			break;
	}
}


unsigned char check_odd_parity(unsigned char x, const unsigned char parity)
{
	unsigned char p;
	
	p = x ^ (x >> 1);
	p ^= (x >> 2);
	p ^= (x >> 4);

	return (p & 1) != parity;
}


/*
	main() - main loop: manage communications with the PS/2 devices
*/
int main(void)
{
	init();
	debug_puts("Hello, World!");
		
	// TODO - ensure that PUD is 0 in SFIOR
	
	// Main loop - handle communication with the CPU.
    while(1)
    {
		if(!(PORTD & nCS))
		{
			// CPU is addressing a bus cycle to us.
			if(!(PORTD & nID))
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
					
					TODO: mutex on register read/write
				*/
				const unsigned char addr = (PORTC & PORTC_ADDR_MASK) >> PORTC_ADDR_SHIFT;

				if(!(PORTC & nUR))			/* Read cycle */
				{
					DO_READ_CYCLE(g_registers[addr]);
				}
				else if(!(PORTC & nUW))		/* Write cycle */
				{
					host_reg_write(addr, DATA_BUS_PIN);
					TERMINATE_BUS_CYCLE();
				}
			}
		}
    }
}
