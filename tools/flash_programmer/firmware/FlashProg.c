#include <avr/io.h>
#include <avr/pgmspace.h>

/*
	Pins
    +---+-----+-----+-----+-----+
	|   |  A  |  B  |  C  |  D  |
	+---+-----+-----+-----+-----+
	| 0 | D0  | A0  | A8  | RXD |
	| 1 | D1  | A1  | A9  | TXD |
	| 2 | D2  | A2  | A10 | RED |
	| 3 | D3  | A3  | A11 | GRN |
	| 4 | D4  | A4  | A12 | CLK |
	| 5 | D5  | A5  | A13 | nCE |
	| 6 | D6  | A6  | A14 | nOE |
	| 7 | D7  | A7  | A15 | nWE |
	+---+-----+-----+-----+-----+

*/

#define RED		(0x04)
#define GRN		(0x08)
#define CLK		(0x10)
#define nCE		(0x20)
#define nOE		(0x40)
#define nWE		(0x80)

#define nop()  __asm__ __volatile__("nop")

#define ERR_OK					('Z')
#define ERR_BAD_CHECKSUM		('C')
#define ERR_TIMEOUT				('T')
#define ERR_UNKNOWN_COMMAND		('?')

#define PROGRAMMER_IDENTITY		("\x08\x08\x19\x78\x00")	// Magic number (+ terminating NUL)
#define PROGRAMMER_IDENTITY_LEN	(5)

#define SOFTWARE_VERSION		("V\x80\x81\x00")			// Version 0.1 (+ terminating NUL)
#define SOFTWARE_VERSION_LEN	(4)

typedef unsigned char uint8;
typedef unsigned long uint32;

const char *const g_sHex = "0123456789abcdef";


const uint32 serial_read_u32()
{
	uint32 val = 0;

	while(!(UCSRA & (1 << RXC))) ;
	val |= UDR;
	val <<= 8;

	while(!(UCSRA & (1 << RXC))) ;
	val |= UDR;
	val <<= 8;

	while(!(UCSRA & (1 << RXC))) ;
	val |= UDR;
	val <<= 8;

	while(!(UCSRA & (1 << RXC))) ;
	val |= UDR;

	return val;
}


char serial_getc(void)
{
	while(!(UCSRA & (1 << RXC))) ;
	return UDR;
}


void serial_putc(const char c)
{
	while(!(UCSRA & (1 << UDRE))) ;
	UDR = c;
}


void serial_puthex8(const uint8 u)
{
	while(!(UCSRA & (1 << UDRE))) ;
	UDR = g_sHex[u >> 4];

	while(!(UCSRA & (1 << UDRE))) ;
	UDR = g_sHex[u & 0xf];
}


void serial_puts(const char *s)
{
	while(*s)
	{
		while(!(UCSRA & (1 << UDRE))) ;
		UDR = *s++;
	}
}


void serial_write(const char *s, int len)
{
	while(len--)
	{
		while(!(UCSRA & (1 << UDRE))) ;
		UDR = *s++;
	}
}


uint8 report_status(const uint8 err)
{
	serial_putc(err);
	return (err == ERR_OK) ? 0 : 1;
}


void bus_init(void)
{
	// Negate nCE, nOE and nWE before the control lines become outputs
	PORTD = nCE | nOE | nWE;

	// Configure data direction register
	DDRA = 0xff;		// Port A:  D0-D7 - all outputs initially
	DDRB = 0xff;		// Port B:  A0-A7 - all outputs
	DDRC = 0xff;		// Port C:  A8-A15 - all outputs
	DDRD = 0xfe;		// Port D:  pd0 input (serial RX) , pd1-pd7 outputs (serial TX, control lines)
}


inline void chip_enable(void)
{
	PORTD |= RED;
	PORTD &= ~nCE;
}


inline void chip_disable(void)
{
	PORTD |= nCE;
	PORTD &= ~RED;
}


const unsigned char data_read(const uint32 address)
{
	uint8 data;

	PORTB = address >> 16;
	PORTD |= CLK;
	PORTD &= ~CLK;

	PORTC = address >> 8;
	PORTB = address & 0xff;

	DDRA = 0x00;	// port A -> inputs
	PORTD &= ~nOE;	// assert nOE
	nop();

	data = PINA;	// read data

	PORTD |= nOE;	// negate nOE and nCE
	DDRA = 0xff;	// port A -> outputs

	return data;
}


void data_write(const uint32 address, const uint8 data)
{
	PORTB = address >> 16;
	PORTD |= CLK;
	PORTD &= ~CLK;

	PORTC = address >> 8;
	PORTB = address & 0xff;

	PORTA = data;

	PORTD &= ~nWE;
	PORTD |= nWE;
}


const uint8 wait(const uint32 address, uint8 data)
{
	uint8 status;
	data &= 0x80;

	do
	{
		status = data_read(address);
//		if((status & 0xa0) == 0x20)
//		{
//			data_write(0, 0xf0);		// reset device
//			return ERR_TIMEOUT;
//		}
	} while((status & 0x80) != data);


	return ERR_OK;
}


const uint8 autoselect(const uint32 address)
{
	uint8 data;

	chip_enable();

	data_write(0x555, 0xaa);
	data_write(0x2aa, 0x55);
	data_write(0x555, 0x90);

	data = data_read(address);

	data_write(0, 0xf0);		// reset device

	chip_disable();

	return data;
}


const uint8 get_manufacturer_id()
{
	return autoselect(0);
}


const uint8 get_device_id()
{
	return autoselect(1);
}


const uint8 get_continuation_id()
{
	return autoselect(3);
}


const uint8 get_sector_protect(const uint8 sector)
{
	return autoselect(((uint32) (sector & 0x7) << 16) | 0x2);
}


const uint8 chip_id()
{
	serial_putc(get_manufacturer_id());
	serial_putc(get_device_id());
	serial_putc(get_continuation_id());
	serial_putc(0x00);

	return ERR_OK;
}


const uint8 chip_erase(void)
{
	uint8 result;

	chip_enable();

	data_write(0x555, 0xaa);
	data_write(0x2aa, 0x55);
	data_write(0x555, 0x80);
	data_write(0x555, 0xaa);
	data_write(0x2aa, 0x55);
	data_write(0x555, 0x10);

	result = wait(0, 0x80);

	chip_disable();

	return result;
}


const uint8 sector_erase(void)
{
	uint8 result, sector;

	sector = serial_getc() - '0';
	if(sector > 7)
		return 1;

	chip_enable();

	data_write(0x555, 0xaa);
	data_write(0x2aa, 0x55);
	data_write(0x555, 0x80);
	data_write(0x555, 0xaa);
	data_write(0x2aa, 0x55);
	data_write((uint32) sector << 16, 0x30);

	result = wait((uint32) sector << 16, 0x80);

	chip_disable();

	return result;
}


const uint8 program_byte(const uint32 address, const uint8 data)
{
	uint8 result;

	chip_enable();

	data_write(0x555, 0xaa);
	data_write(0x2aa, 0x55);
	data_write(0x555, 0xa0);

	data_write(address, data);

	result = wait(address, data);

	chip_disable();

	return result;
}


const uint8 block_program()
{
	uint8 result, buffer[256];
	uint32 i;
	uint32 offset, checksum_read, checksum_local;

	// read offset (address >> 8)
	offset = serial_read_u32();

	// read 256 bytes
	checksum_local = 0;
	for(i = 0; i <= 255; ++i)
	{
		buffer[i] = serial_getc();
		checksum_local += buffer[i];
	}

	// read checksum
	checksum_read = serial_read_u32();

	// checksum OK?
	if(checksum_read != checksum_local)
		return ERR_BAD_CHECKSUM;

	chip_enable();

	// program block
	for(i = 0; i <= 255; ++i)
	{
		data_write(0x555, 0xaa);
		data_write(0x2aa, 0x55);
		data_write(0x555, 0xa0);
		data_write(offset, buffer[i]);

		result = wait(offset++, buffer[i]);
		if(result != ERR_OK)
		{
			chip_disable();
			return result;
		}
	}

	chip_disable();

	return ERR_OK;		// success
}


const uint8 block_read()
{
	uint32 offset, i = 255;

	// read offset (i.e. base addr. for read)
	offset = serial_read_u32();

	chip_enable();

	do
		serial_putc(data_read(offset++));
	while(i--);

	chip_disable();

	return ERR_OK;
}



int main()
{
	bus_init();

	// Configure serial port
	
	// Baud rate = 115200 (UBRR = 3 for 7.3728MHz sysclk)
	UBRRH = 0;
	UBRRL = 3;

	// Baud rate = 230400(UBRR = 1 for 7.3728MHz sysclk)
//	UBRRH = 0;
//	UBRRL = 1;

	// Enable receiver and transmitter; set 8 bits/word, no parity, 1 stop bit
	UCSRB = (1 << RXEN) | (1 << TXEN);
	UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);

	// Main loop.  Wait for instructions.
	PORTD |= GRN;
	while(1)
	{
		// Read char from serial port
		switch(serial_getc())
		{
			case 'E':		// E = erase sector
				// read sector num
				while(!(UCSRA & (1 << RXC))) ;
				report_status(sector_erase());
				break;

			case 'C':		// C = erase chip
				report_status(chip_erase());
				break;

			case 'I':		// I = identify
				chip_id();
				break;

			case 'P':		// P = program
				report_status(block_program());
				break;

			case 'R':		// R = read block
			 	block_read();
				break;

			case 'V':		// V = version
				serial_write(SOFTWARE_VERSION, SOFTWARE_VERSION_LEN);
				break;

			case '?':
				serial_write(PROGRAMMER_IDENTITY, PROGRAMMER_IDENTITY_LEN);
				break;

			default:
				report_status(ERR_UNKNOWN_COMMAND);
				break;
		}
	}
}
