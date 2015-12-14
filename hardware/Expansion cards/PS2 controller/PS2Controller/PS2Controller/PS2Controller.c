/*
	PS2Controller.c - PS/2 keyboard & mouse controller implementation
	
	This module contains firmware for an m68k-bus peripheral controller.
	
	
	(c) Stuart Wallace <stuartw@atom.net>, November 2015.
 */

#define WITH_DEBUGGING				/* Enable debugging */

#include "PS2Controller.h"

ctx_t ctx_kb, ctx_mouse;
u8 g_registers[8];


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
	ctx_kb.state_data		= ds_idle;
	ctx_kb.state_cmd		= cs_rx_data;		/* TODO: change to cs_ignore */
	ctx_kb.port				= &KB_PORT;
	ctx_kb.pin				= &KB_PIN;
	ctx_kb.ddr				= &KB_DDR;
	ctx_kb.data_pin			= KB_DATA;
	ctx_kb.data_regnum		= REG_KB_DATA;
	ctx_kb.command			= CMD_NONE;
	ctx_kb.data_present		= 0;
	
	ctx_mouse.state_data	= ds_idle;
	ctx_mouse.state_cmd		= cs_rx_data;		/* TODO: change to cs_ignore */
	ctx_mouse.port			= &MOUSE_PORT;
	ctx_mouse.pin			= &MOUSE_PIN;
	ctx_mouse.ddr			= &MOUSE_DDR;
	ctx_mouse.data_pin		= MOUSE_DATA;
	ctx_mouse.data_regnum	= REG_MOUSE_DATA;
	ctx_mouse.command		= CMD_NONE;
	ctx_mouse.data_present	= 0;

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
	g_registers[reg] = data;

	/* TODO: act on changes to registers as appropriate */
}


/*
	process_data_kb() - process a byte of data received from the keyboard.
*/
void process_data_kb()
{
	/* TODO - maybe pull clock low to prevent device sending more data until we're finished? */
	u8 data = ctx_kb.data;
	
	ctx_kb.data_present = 0;
	
	if(ctx_kb.parity_received != ctx_kb.parity_calc)
	{
		/* Parity error */
		g_registers[REG_STATUS] |= STATUS_KBPARERR;
		if((g_registers[REG_INTCFG] & (INTCFG_IE | INTCFG_KBPARERRIE))
			== (INTCFG_IE | INTCFG_KBPARERRIE))
		{
			/* TODO: assert nIRQ */
		}

		debug_putc('!');
		return;
	}
	
	/* Process received data */
	switch(ctx_kb.state_cmd)
	{
		case cs_ignore:
			break;
		
		case cs_rx_data:
			if(data == 0xe0)
				ctx_kb.flags |= KB_EXT;			/* FIXME - nothing unsets this flag */
			else if(data == 0xf0)
				ctx_kb.flags |= KB_RELEASE;		/* FIXME - nothing unsets this flag */
			else
			{
				if(ctx_kb.flags & KB_RELEASE)
					data |= 0x80;

				g_registers[REG_KB_DATA] = data;
				g_registers[REG_STATUS] |= STATUS_KBRX;
				
				if((g_registers[REG_INTCFG] & (INTCFG_IE | INTCFG_KBRXIE))
					== (INTCFG_IE | INTCFG_KBRXIE))
				{
					/* TODO: assert nIRQ */
				}
				
				ctx_kb.flags &= ~(KB_EXT | KB_RELEASE);
				
				debug_puthexb(data);
				debug_putc('\n');
			}
			break;
	}
}


/*
	process_data_mouse() - process a byte of data received from the mouse.
*/
void process_data_mouse()
{
	/* TODO - maybe pull clock low to prevent device sending more data until we're finished? */
	u8 data = ctx_mouse.data;
	
	ctx_mouse.data_present = 0;
	
	if(ctx_mouse.parity_received != ctx_mouse.parity_calc)
	{
		/* Parity error */
		g_registers[REG_STATUS] |= STATUS_MOUSEPARERR;
		if((g_registers[REG_INTCFG] & (INTCFG_IE | INTCFG_MOUSEPARERRIE))
			== (INTCFG_IE | INTCFG_MOUSEPARERRIE))
		{
			/* TODO: assert nIRQ */
		}

		debug_putc('!');
		return;
	}

	debug_puthexb(data);
}


/*
	process_clock_edge() - handle a clock transition on a PS/2 port.
	This function deals with the transmission and receipt of data on the ports.
*/
void process_clock_edge(ctx_t *ctx)
{
	switch(++(ctx->state_data))
	{
		case ds_idle:		/* Unreachable					*/
		case ds_tx_busrq:	/* Unreachable (handled in ISR)	*/
			break;

		case ds_rx_start:
			ctx->parity_calc = 1;		/* because odd parity */
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
			if(*ctx->pin & ctx->data_pin)
			{
				ctx->data |= 0x80;
				ctx->parity_calc ^= 1;
			}
			break;
		
		case ds_rx_parity:
			ctx->parity_received = (*ctx->pin & ctx->data_pin) ? 1 : 0;
			break;
		
		case ds_rx_stop:
			ctx->state_data = ds_idle;
			ctx->data_present = 1;
			break;
			
		case ds_tx_start:
			SET_LOW(*ctx->port, ctx->data_pin);
			ctx->parity_calc = 1;		/* because odd parity */
			break;

		case ds_tx_d0:
		case ds_tx_d1:
		case ds_tx_d2:
		case ds_tx_d3:
		case ds_tx_d4:
		case ds_tx_d5:
		case ds_tx_d6:
		case ds_tx_d7:
			if(ctx->command & 0x1)
			{
				SET_HIGH(*ctx->port, ctx->data_pin);
				ctx->parity_calc ^= 1;
			}
			else
				SET_LOW(*ctx->port, ctx->data_pin);
			ctx->command >>= 1;
			break;
		
		case ds_tx_parity:
			if(ctx->parity_calc)
				SET_HIGH(*ctx->port, ctx->data_pin);
			else
				SET_LOW(*ctx->port, ctx->data_pin);
		
		case ds_tx_stop:
			SET_HIGH(*ctx->port, ctx->data_pin);
			SET_INPUT(*ctx->ddr, ctx->data_pin);
			break;
			
		case ds_tx_ack:
			ctx->state_data = ds_idle;
			/* TODO: assert STATUS_xxTXDONE */
			/* TODO: assert nIRQ if CONFIG_IE and CONFIG_xxTXIE are both set */
			break;
	}
}


/*
	ISR for INT0 - called on falling edges of keyboard clock; handles incoming data.
*/
ISR(INT0_vect)
{
	process_clock_edge(&ctx_kb);
}


/*
	ISR for INT1 - called on falling edges of mouse clock; handles incoming data.
*/
ISR(INT1_vect)
{
	process_clock_edge(&ctx_mouse);
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
	ctx_kb.state_data = ds_idle;		/* Just in case */
	
	/* Start keyboard command transmission */
	KB_CLK_SET_OUTPUT();		/* Set keyboard clock line to output		*/
	KB_CLK_SET_LOW();			/* Pull clock line low						*/

	/* Generate an interrupt after 60us */
	TCNT0 = 196;				/* 196 = 256 - 60, ie. overflow (and interrupt) after 60 counts */
	TCCR0 = _BV(CS01);			/* Set timer clock source to f(clk)/8 (=1MHz)					*/

	ctx_kb.state_data = ds_tx_busrq;
}


void bus_request_mouse()
{
	MOUSE_IRQ_DISABLE();
	ctx_mouse.state_data = ds_idle;		/* Just in case */
	
	/* Start mouse command transmission */
	MOUSE_CLK_SET_OUTPUT();		/* Set mouse clock line to output			*/
	MOUSE_CLK_SET_LOW();		/* Pull clock line low						*/
	
	/* Generate an interrupt after 60us */
	TCNT2 = 196;				/* 196 = 256 - 60, ie. overflow (and interrupt) after 60 counts */
	TCCR2 = _BV(CS01);			/* Set timer clock source to f(clk)/8 (=1MHz)					*/
	
	ctx_mouse.state_data = ds_tx_busrq;
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
					ku8 data = DATA_BUS_PIN;
					TERMINATE_BUS_CYCLE();
					
					host_reg_write(addr, data);
				}
			}
		}
		
		/* If data has been received from the keyboard or the mouse, process it */
		if(ctx_kb.data_present)
			process_data_kb();
			
		if(ctx_mouse.data_present)
			process_data_mouse();

		/* If the command register contains a command, and the receiver is idle, send the command */
		if((ctx_kb.command != CMD_NONE) && (ctx_kb.state_data == ds_idle))
			bus_request_keyboard();		/* Start keyboard command transmission */

		if((ctx_mouse.command != CMD_NONE) && (ctx_mouse.state_data == ds_idle))
			bus_request_mouse();		/* Start mouse command transmission */
    }
}
