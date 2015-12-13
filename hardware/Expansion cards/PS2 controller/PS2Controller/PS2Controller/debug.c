/*
	debug.c - debugging functions
	
	
	(c) Stuart Wallace <stuartw@atom.net>, December 2015.
*/ 

#include <avr/io.h>
//#include "debug.h"


/*
	debug_init() - initialise debug console.  NOTE: this configures PD1 and PD0 as serial IO lines.
*/
void debug_init()
{
	// Apply oscillator calibration
	// NOTE: this is a device-specific value and will vary from chip to chip!
	OSCCAL = 0x9b;
	
	// Configure serial port
	DDRD |= _BV(1);		// Set PD0 (TXD) to output
	DDRD &= ~(_BV(0));	// Set PD1 (RXD) to input
		
	UBRRH = 0;			// } 38400 baud at 8MHz clk
	UBRRL = 12;			// }

	UCSRC = _BV(URSEL) | (3 << UCSZ0);
	UCSRB = _BV(RXEN) | _BV(TXEN);	
}


/*
	debug_putc() - write char c to the debug console.
*/
void debug_putc(const char c)
{
	while(!(UCSRA & _BV(UDRE)))
		;
		
	UDR = c;
}


/*
	debug_puts() - write string s to the debug console.
*/
void debug_puts(const char *s)
{
	for(; *s; ++s)
	{
		while(!(UCSRA & _BV(UDRE)))
			;
		
		UDR = *s;
	}
}


/*
	debug_puthexb() - write the supplied byte to the debug console in hex form, e.g. "3c".
*/
void debug_puthexb(const char c)
{
	static const char * const hex = "0123456789abcdef";
	
	debug_putc(hex[(c & 0xf0) >> 4]);
	debug_putc(hex[c & 0x0f]);
}
