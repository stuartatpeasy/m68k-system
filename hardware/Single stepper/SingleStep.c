#include <avr/io.h>

#define nDTACK (0x20)
#define nLED   (0x10)

#define nop()  __asm__ __volatile__("nop")


int main()
{
	volatile long i;

	// Set all PORTC pins high
	PORTC = 0xff;

	// Port C: all outputs
	DDRC = 0xff;

	// Port B: all inputs
	DDRB = 0x00;

	while(1)
	{
		if(PINB & 0x02)
		{
			PORTC &= ~(nDTACK | nLED);
			nop();
			PORTC |= nDTACK;

			for(i = 0; i < 100000; ++i) ;
			PORTC |= nLED;

			while(PINB & 0x02)
				for(i = 0; i < 100000; ++i) ;
		}
	}

	return 0;
}
