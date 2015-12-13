/*
	PS2Controller.c - PS/2 keyboard & mouse controller implementation
	
	This module contains firmware for an m68k-bus peripheral controller.
	
	
	(c) Stuart Wallace <stuartw@atom.net>, November 2015.
 */

#define WITH_DEBUGGING				/* Enable debugging */

#include "PS2Controller.h"

ctx_t kb_ctx, mouse_ctx;
u8 g_registers[16];


/*
	init() - initialise the chip following a reset
*/
void init(void)
{
	u8 i;
	
	DDRD			= PORTD_OUTPUTS;
	PORTD			= PORTD_PULLUPS;

	DDRC			= PORTC_OUTPUTS;
	PORTC			= PORTC_PULLUPS;

	DATA_BUS_DDR	= DATA_BUS_OUTPUTS;
	DATA_BUS		= DATA_BUS_PULLUPS;

	/* Initialise register values */	
	for(i = 0; i < sizeof(g_registers) / sizeof(g_registers[0]); ++i)
		g_registers[i] = 0;
		
	/* Initialise mouse and keyboard context */
	kb_ctx.state_data		= ds_idle;
	kb_ctx.state_cmd		= cs_rx_data;		/* TODO: change to cs_ignore */
	kb_ctx.data				= 0;
	kb_ctx.parity_calc		= 0;
	kb_ctx.data_regnum		= REG_KB_DATA;
	kb_ctx.command			= CMD_NONE;
	
	mouse_ctx.state_data	= ds_idle;
	mouse_ctx.state_cmd		= cs_rx_data;		/* TODO: change to cs_ignore */
	mouse_ctx.data			= 0;
	mouse_ctx.parity_calc	= 0;
	mouse_ctx.data_regnum	= REG_MOUSE_DATA;
	mouse_ctx.command		= CMD_NONE;

	/* Enable keyboard and mouse interrupts on falling edge */
	set_irq_edge(irq_keyboard_clk, irq_edge_falling);
	set_irq_edge(irq_mouse_clk, irq_edge_falling);
	
	KB_IRQ_ENABLE();
	MOUSE_IRQ_ENABLE();
	
	debug_init();
	
	sei();
}


/*
	set_irq_edge() - configure interrupts to trigger on the rising (IRQ_EDGE_RISING) or falling
	(IRQ_EDGE_FALLING) edge of INT1 and INT0 inputs.
*/
void set_irq_edge(const irq_t irq, const irq_edge_t edge)
{
	if(irq == irq_keyboard_clk)
	{
		if(edge == irq_edge_falling)
			INT0_SET_FALLING_EDGE();
		else if(edge == irq_edge_rising)
			INT0_SET_RISING_EDGE();
	}
	else if(irq == irq_mouse_clk)
	{
		if(edge == irq_edge_falling)
			INT1_SET_FALLING_EDGE();
		else if(edge == irq_edge_rising)
			INT1_SET_RISING_EDGE();
	}
}


/*
	host_reg_write() - handle a register write by the host.
*/
void host_reg_write(ku8 reg, ku8 data)
{
	cli();
	g_registers[reg] = data;
	sei();
	/* TODO: act on changes to registers as appropriate */
}


void kb_send_command(u8 cmd)
{
	u8 i;
	
	// TODO: generate parity of cmd

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


void receive_bit(ctx_t *ctx, u8 bit)
{
	switch(++(ctx->state_data))
	{
		case ds_idle:		/* Unreachable					*/
		case ds_tx_busrq:	/* Unreachable (handled in ISR)	*/
			break;

		case ds_rx_start:
			/* TODO: maybe flag start of data reception? */
			break;

		case ds_rx_d0:
		case ds_rx_d1:
		case ds_rx_d2:
		case ds_rx_d3:
		case ds_rx_d4:
		case ds_rx_d5:
		case ds_rx_d6:
		case ds_rx_d7:
			ctx->data >>= 1;
			if(bit)
			{
				ctx->data |= 0x80;
				ctx->parity_calc ^= 1;
			}
			break;
		
		case ds_rx_parity:
			ctx->parity_received = bit ? 1 : 0;
			break;
		
		case ds_rx_stop:
			if(ctx->parity_received != ctx->parity_calc)
			{
				debug_puthexb(kb_ctx.data);			/* TODO: process received data */
				g_registers[ctx->data_regnum] = ctx->data;
			}
			else
			{
				/* Parity error - drop data */
				debug_putc('!');
			}
			debug_putc('\n');

			ctx->state_data = ds_idle;
			ctx->parity_calc = 0;
			break;
	}
}


void transmit_bit(ctx_t *ctx, u8 port, u8 pin, u8 bit)
{

}


/*
	process_clock_edge() - handle a clock transition on a PS/2 port.
*/
void process_clock_edge(ctx_t *ctx, u8 bit)
{
	switch(ctx->state_cmd)
	{
		default:			/* FIXME remove */
		case cs_rx_data:
			receive_bit(ctx, bit);
			break;
	}
}


/*
	ISR for INT0 - called on falling edges of keyboard clock; handles incoming data.
*/
ISR(INT0_vect)
{
	process_clock_edge(&kb_ctx, PIND & KB_DATA);
}


/*
	ISR for INT1 - called on falling edges of mouse clock; handles incoming data.
*/
ISR(INT1_vect)
{
	process_clock_edge(&mouse_ctx, PIND & MOUSE_DATA);
}


/*
	ISR for timer 0 overflow - handles keyboard data-transmission bus requests.
*/
ISR(TIMER0_OVF_vect)
{
	/* Disable timer 0 */
	SET_LOW(TIMSK, _BV(TOIE0));
	SET_LOW(TCCR0, _BV(CS02) | _BV(CS01) | _BV(CS00));

	KB_DATA_SET_LOW();			/* Pull the keyboard data line low		*/
	KB_DATA_SET_OUTPUT();		/* Set keyboard data line as output		*/
	KB_CLK_SET_HIGH();			/* Pull the keyboard clock line high	*/
	KB_CLK_SET_INPUT();			/* Set keyboard clock as input			*/

	/* Re-enable interrupts on rising edges of keyboard clock */
	set_irq_edge(irq_keyboard_clk, irq_edge_rising);
	KB_IRQ_ENABLE();
}


/*
	ISR for timer 2 overflow - handles mouse data-transmission bus requests.
*/
ISR(TIMER2_OVF_vect)
{
	SET_LOW(TIMSK, _BV(TOIE2));
	SET_LOW(TCCR2, _BV(CS02) | _BV(CS01) | _BV(CS00));	/* Check this */
	
	MOUSE_DATA_SET_LOW();		/* Pull the mouse data line low		*/
	MOUSE_DATA_SET_OUTPUT();	/* Set mouse data line as output	*/
	MOUSE_CLK_SET_HIGH();		/* Pull the mouse clock line high	*/
	MOUSE_CLK_SET_INPUT();		/* Set mouse clock as input			*/

	/* Re-enable interrupts on rising edges of mouse clock	*/
	set_irq_edge(irq_mouse_clk, irq_edge_rising);
	MOUSE_IRQ_ENABLE();
}


void bus_request_keyboard()
{
	KB_IRQ_DISABLE();
	kb_ctx.state_data = ds_idle;		/* Just in case */
	
	/* Start keyboard command transmission */
	KB_CLK_SET_OUTPUT();		/* Set keyboard clock line to output		*/
	KB_CLK_SET_LOW();			/* Pull clock line low						*/

	/* Generate an interrupt after 60us */
	TCNT0 = 196;				/* 196 = 256 - 60, ie. overflow (and interrupt) after 60 counts */
	TCCR0 = _BV(CS01);			/* Set timer clock source to f(clk)/8 (=1MHz)					*/

	kb_ctx.state_data = ds_tx_busrq;
}


void bus_request_mouse()
{
	MOUSE_IRQ_DISABLE();
	mouse_ctx.state_data = ds_idle;		/* Just in case */
	
	/* Start mouse command transmission */
	MOUSE_CLK_SET_OUTPUT();		/* Set mouse clock line to output			*/
	MOUSE_CLK_SET_LOW();		/* Pull clock line low						*/
	
	/* Generate an interrupt after 60us */
	TCNT2 = 196;				/* 196 = 256 - 60, ie. overflow (and interrupt) after 60 counts */
	TCCR2 = _BV(CS01);			/* Set timer clock source to f(clk)/8 (=1MHz)					*/
	
	mouse_ctx.state_data = ds_tx_busrq;
}


/*
	main() - main loop: manage communications with the host
*/
int main(void)
{
	init();

    while(1)
    {
		if(!(PORTD & nCS))		/* Host bus cycle in progress */
		{
			if(!(PORTD & nID))		/* nID asserted: place device ID on data bus and complete cycle */
			{
				DO_READ_CYCLE(PERIPHERAL_ID);
			}
			else					/* nID negated: register read/write cycle */
			{
				ku8 addr = GET_ADDRESS();

				if(PORTC & nW)			/* nW negated - this is a read cycle */
				{
					DO_READ_CYCLE(g_registers[addr]);
				}
				else					/* nW asserted - this is a write cycle */
				{
					host_reg_write(addr, DATA_BUS_PIN);
					TERMINATE_BUS_CYCLE();
				}
			}
		}

		if((kb_ctx.command != CMD_NONE) && (kb_ctx.state_data == ds_idle))
			bus_request_keyboard();		/* Start keyboard command transmission */

		if((mouse_ctx.command != CMD_NONE) && (mouse_ctx.state_data == ds_idle))
			bus_request_mouse();		/* Start mouse command transmission */
    }
}
