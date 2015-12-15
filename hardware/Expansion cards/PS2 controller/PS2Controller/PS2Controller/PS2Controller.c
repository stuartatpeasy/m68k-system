/*
	PS2Controller.c - PS/2 keyboard & mouse controller implementation
	
	This module contains firmware for an m68k-bus peripheral controller.
	
	
	(c) Stuart Wallace <stuartw@atom.net>, November 2015.
 */

#define WITH_DEBUGGING				/* Enable debugging */

#include "PS2Controller.h"

ctx_t ctx_kb, ctx_mouse;
u8 registers[8];


/*
	init() - initialise the chip following a reset
*/
void init(void)
{
	u8 i;
	
	cli();
	
	/* Configure ports */
	DDRD			= PORTD_OUTPUTS;
	PORTD			= PORTD_PULLUPS;

	DDRC			= PORTC_OUTPUTS;
	PORTC			= PORTC_PULLUPS;

	DATA_BUS_DDR	= DATA_BUS_OUTPUTS;
	DATA_BUS		= DATA_BUS_PULLUPS;

	/* Initialise register values */	
	for(i = 0; i < sizeof(registers) / sizeof(registers[0]); ++i)
		registers[i] = 0;
		
	/* Initialise mouse and keyboard context */
	ctx_kb.state_data		= ds_idle;
	ctx_kb.port				= &KB_PORT;
	ctx_kb.pin				= &KB_PIN;
	ctx_kb.ddr				= &KB_DDR;
	ctx_kb.data_pin			= KB_DATA;
	ctx_kb.data_regnum		= REG_KB_DATA;
	ctx_kb.command			= CMD_NONE;
	ctx_kb.flag_rx			= FLAG_KBRX;
	ctx_kb.flag_tx			= FLAG_KBTXDONE;
	ctx_kb.flag_ovf			= FLAG_KBOVF;
	ctx_kb.flag_parerr		= FLAG_KBPARERR;
	ctx_kb.fifo.rd			= 0;
	ctx_kb.fifo.wr			= 0;
	
	ctx_mouse.state_data	= ds_idle;
	ctx_mouse.port			= &MOUSE_PORT;
	ctx_mouse.pin			= &MOUSE_PIN;
	ctx_mouse.ddr			= &MOUSE_DDR;
	ctx_mouse.data_pin		= MOUSE_DATA;
	ctx_mouse.data_regnum	= REG_MOUSE_DATA;
	ctx_mouse.command		= CMD_NONE;
	ctx_mouse.flag_rx		= FLAG_MOUSERX;
	ctx_mouse.flag_tx		= FLAG_MOUSETXDONE;
	ctx_mouse.flag_ovf		= FLAG_MOUSEOVF;
	ctx_mouse.flag_parerr	= FLAG_MOUSEPARERR;
	ctx_mouse.fifo.rd		= 0;
	ctx_mouse.fifo.wr		= 0;

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
	registers[reg] = data;

	/* TODO: act on changes to registers as appropriate */
}


/*
	process_data() - process a byte of data received from the keyboard or the mouse.
*/
void process_data(ctx_t *ctx)
{
	if(ctx->parity_received == ctx->parity_calc)
	{
		ku8 data = ctx->data,
			fifo_wrnext = ctx->fifo.wr + 1;

		/* Place received data in FIFO */
		if(fifo_wrnext != ctx->fifo.rd)
		{
			ctx->fifo.data[ctx->fifo.wr] = data;
			ctx->fifo.wr = fifo_wrnext;
			debug_puthexb(data);
			debug_putc('\n');

			registers[REG_STATUS] |= ctx->flag_rx;
			if(registers[REG_INTCFG] & ctx->flag_rx)
				HOST_IRQ_ASSERT();
		}
		else
		{
			/* FIFO is full; drop data */
			registers[REG_STATUS] |= ctx->flag_ovf;
			if(registers[REG_INTCFG] & ctx->flag_ovf)
				HOST_IRQ_ASSERT();
		}
	}
	else
	{
		/* Parity error */
		registers[REG_STATUS] |= ctx->flag_parerr;
		if(registers[REG_INTCFG] & ctx->flag_parerr)
			HOST_IRQ_ASSERT();

		debug_putc('!');
	}
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
			process_data(ctx);
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
			registers[REG_STATUS] |= ctx->flag_tx;
			if(registers[REG_INTCFG] & ctx->flag_tx)
				HOST_IRQ_ASSERT();
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


void start_tx_keyboard()
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


void start_tx_mouse()
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
					DO_READ_CYCLE(registers[addr]);
				}
				else					/* nW asserted - this is a write cycle */
				{
					ku8 data = DATA_BUS_PIN;
					TERMINATE_BUS_CYCLE();
					
					host_reg_write(addr, data);
				}
			}
		}
		
		/* If the command register contains a command, and the receiver is idle, send the command */
		if((ctx_kb.command != CMD_NONE) && (ctx_kb.state_data == ds_idle))
			start_tx_keyboard();

		if((ctx_mouse.command != CMD_NONE) && (ctx_mouse.state_data == ds_idle))
			start_tx_mouse();
    }
}
