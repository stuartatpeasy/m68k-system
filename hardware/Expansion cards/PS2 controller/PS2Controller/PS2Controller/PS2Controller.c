/*
	PS2Controller.c - PS/2 keyboard & mouse controller implementation
	
	This module contains firmware for an m68k-bus peripheral controller.
	
	
	(c) Stuart Wallace <stuartw@atom.net>, November 2015.
 */

//#define WITH_DEBUGGING				/* Enable debugging */

#include "PS2Controller.h"

volatile ctx_t ctx_kb, ctx_mouse;
vu8 registers[8];


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
	DATA_BUS_PORT	= DATA_BUS_PULLUPS;
	
	/* Disable timers */
	TCCR0	= 0;
	TCCR1A	= 0;
	TCCR1B	= 0;
	TCCR2	= 0;
	
	/* TODO: enable timer interrupts, I think */
	
	/* Initially disable power to keyboard and mouse */
	SET_LOW(PWR_PORT, PWR_KB | PWR_MOUSE);

	/* Initialise register values */	
	for(i = 0; i < sizeof(registers) / sizeof(registers[0]); ++i)
		registers[i] = 0;
		
	/* Initialise mouse and keyboard context */
	ctx_kb.state				= state_idle;
	ctx_kb.port					= &KB_PORT;
	ctx_kb.pin					= &KB_PIN;
	ctx_kb.ddr					= &KB_DDR;
	ctx_kb.data_pin				= KB_DATA;
	ctx_kb.data_regnum			= REG_KB_DATA;
	ctx_kb.command				= CMD_NONE;
	ctx_kb.command_pending		= 0;
	ctx_kb.start_tx_fn			= start_tx_keyboard;
	ctx_kb.flag_rx				= FLAG_KBRX;
	ctx_kb.flag_tx				= FLAG_KBTXDONE;
	ctx_kb.flag_ovf				= FLAG_KBOVF;
	ctx_kb.flag_parerr			= FLAG_KBPARERR;
	ctx_kb.irq					= irq_keyboard_clk;
	ctx_kb.fifo.rd				= 0;
	ctx_kb.fifo.wr				= 0;
	
	ctx_mouse.state				= state_idle;
	ctx_mouse.port				= &MOUSE_PORT;
	ctx_mouse.pin				= &MOUSE_PIN;
	ctx_mouse.ddr				= &MOUSE_DDR;
	ctx_mouse.data_pin			= MOUSE_DATA;
	ctx_mouse.data_regnum		= REG_MOUSE_DATA;
	ctx_mouse.command			= CMD_NONE;
	ctx_mouse.command_pending	= 0;
	ctx_mouse.start_tx_fn		= start_tx_mouse;
	ctx_mouse.flag_rx			= FLAG_MOUSERX;
	ctx_mouse.flag_tx			= FLAG_MOUSETXDONE;
	ctx_mouse.flag_ovf			= FLAG_MOUSEOVF;
	ctx_mouse.flag_parerr		= FLAG_MOUSEPARERR;
	ctx_mouse.irq				= irq_mouse_clk;
	ctx_mouse.fifo.rd			= 0;
	ctx_mouse.fifo.wr			= 0;

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
	process_data() - process a byte of data received from the keyboard or the mouse.
*/
void process_data(volatile ctx_t *ctx)
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
			if((registers[REG_CFG] & CFG_IE) && (registers[REG_INT_CFG] & ctx->flag_rx))
				HOST_IRQ_ASSERT();
		}
		else
		{
			/* FIFO is full; drop data */
			registers[REG_STATUS] |= ctx->flag_ovf;
			if((registers[REG_CFG] & CFG_IE) && (registers[REG_INT_CFG] & ctx->flag_ovf))
				HOST_IRQ_ASSERT();
		}
	}
	else
	{
		/* Parity error */
		registers[REG_STATUS] |= ctx->flag_parerr;
		if((registers[REG_CFG] & CFG_IE) && (registers[REG_INT_CFG] & ctx->flag_parerr))
			HOST_IRQ_ASSERT();

		debug_putc('!');
	}
}


/*
	process_clock_edge() - handle a clock transition on a PS/2 port.
	This function deals with the transmission and receipt of data on the ports.
*/
void process_clock_edge(volatile ctx_t *ctx)
{
	switch(++(ctx->state))
	{
		case state_idle:		/* Unreachable					*/
		case state_tx_busrq:	/* Unreachable (handled in ISR)	*/
			break;

		case state_rx_start:
			ctx->parity_calc = 1;		/* because odd parity */
			break;

		case state_rx_d0:
		case state_rx_d1:
		case state_rx_d2:
		case state_rx_d3:
		case state_rx_d4:
		case state_rx_d5:
		case state_rx_d6:
		case state_rx_d7:
			ctx->data >>= 1;
			if(*ctx->pin & ctx->data_pin)
			{
				ctx->data |= 0x80;
				ctx->parity_calc ^= 1;
			}
			break;
		
		case state_rx_parity:
			ctx->parity_received = (*ctx->pin & ctx->data_pin) ? 1 : 0;
			break;
		
		case state_rx_stop:
			process_data(ctx);
			if(ctx->command_pending)
				ctx->start_tx_fn();
			else
				ctx->state = state_idle;
			break;
			
		case state_tx_start:
			SET_LOW(*ctx->port, ctx->data_pin);
			ctx->parity_calc = 1;		/* because odd parity */
			break;

		case state_tx_d0:
		case state_tx_d1:
		case state_tx_d2:
		case state_tx_d3:
		case state_tx_d4:
		case state_tx_d5:
		case state_tx_d6:
		case state_tx_d7:
			if(ctx->command & 0x1)
			{
				SET_HIGH(*ctx->port, ctx->data_pin);
				ctx->parity_calc ^= 1;
			}
			else
				SET_LOW(*ctx->port, ctx->data_pin);
			ctx->command >>= 1;
			break;
		
		case state_tx_parity:
			if(ctx->parity_calc)
				SET_HIGH(*ctx->port, ctx->data_pin);
			else
				SET_LOW(*ctx->port, ctx->data_pin);
			break;
		
		case state_tx_stop:
			SET_HIGH(*ctx->port, ctx->data_pin);
			SET_INPUT(*ctx->ddr, ctx->data_pin);
			break;
			
		case state_tx_ack:
			registers[REG_STATUS] |= ctx->flag_tx;
			ctx->state = state_idle;
			if((registers[REG_CFG] & CFG_IE) && (registers[REG_INT_CFG] & ctx->flag_tx))
				HOST_IRQ_ASSERT();
				
			if(ctx->command_pending)
				ctx->start_tx_fn();
			break;
	}
	
	registers[REG_UNUSED] = ctx->state;
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

	/* Re-enable keyboard interrupts */
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

	/* Re-enable mouse interrupts */
	MOUSE_IRQ_ENABLE();
}


/*
	start_tx_keyboard() - start transmitting a byte to the keyboard.
*/
void start_tx_keyboard()
{
	KB_IRQ_DISABLE();
	ctx_kb.command_pending = 0;
	
	/* Start keyboard command transmission */
	KB_CLK_SET_OUTPUT();			/* Set keyboard clock line to output				*/
	KB_CLK_SET_LOW();				/* Pull PS/2 keyboard clock line low				*/

	/* Generate an interrupt after 150us */
	TCNT0 = 106;					/* 106 = 256 - 150, ie. overflow after 150 counts	*/
	SET_HIGH(TIMSK, _BV(TOIE0));	/* Enable interrupt on timer 0 overflow				*/
	TCCR0 = _BV(CS01);				/* Set timer clock source to f(clk)/8 (=1MHz)		*/

	ctx_kb.state = state_tx_busrq;
}


/*
	start_tx_mouse() - start transmitting a byte to the mouse.
*/
void start_tx_mouse()
{
	MOUSE_IRQ_DISABLE();
	ctx_mouse.command_pending = 0;
	
	/* Start mouse command transmission */
	MOUSE_CLK_SET_OUTPUT();			/* Set mouse clock line to output					*/
	MOUSE_CLK_SET_LOW();			/* Pull clock line low								*/
	
	/* Generate an interrupt after 150us */
	TCNT2 = 106;					/* 106 = 256 - 150, ie. overflow after 150 counts	*/
	SET_HIGH(TIMSK, _BV(TOIE2));	/* Enable interrupt on timer 2 overflow				*/
	TCCR2 = _BV(CS01);				/* Set timer clock source to f(clk)/8 (=1MHz)		*/
	
	ctx_mouse.state = state_tx_busrq;
}


/*
	main() - main loop: manage communications with the host
*/
int main(void)
{
	init();

    while(1)
    {
		if(!(PIND & nCS))		/* Host bus cycle in progress */
		{
			if(!(PIND & nID))		/* nID asserted: place device ID on data bus and complete cycle */
			{
				DO_READ_CYCLE(PERIPHERAL_ID);
			}
			else					/* nID negated: register read/write cycle */
			{
				ku8 addr = GET_ADDRESS();

				if(PINC & nW)			/* nW negated - this is a read cycle */
				{
					switch(addr)
					{
						case REG_KB_DATA:
							cli();
							if(ctx_kb.fifo.rd == ctx_kb.fifo.wr)
							{
								DO_READ_CYCLE(0);		/* FIFO is empty */
								registers[REG_STATUS] &= ~FLAG_KBRX;
							}
							else
							{
								DO_READ_CYCLE(ctx_kb.fifo.data[ctx_kb.fifo.rd++]);
								if(ctx_kb.fifo.rd == ctx_kb.fifo.wr)
									registers[REG_STATUS] &= ~FLAG_KBRX;
							}
							sei();
							break;
							
						case REG_MOUSE_DATA:
							cli();
							if(ctx_mouse.fifo.rd == ctx_mouse.fifo.wr)
							{
								DO_READ_CYCLE(0);		/* FIFO is empty */
								registers[REG_STATUS] &= ~FLAG_MOUSERX;
							}
							else
							{
								DO_READ_CYCLE(ctx_mouse.fifo.data[ctx_mouse.fifo.rd++]);
								if(ctx_mouse.fifo.rd == ctx_mouse.fifo.wr)
									registers[REG_STATUS] &= ~FLAG_KBRX;
							}
							sei();
							break;
							
						default:
							DO_READ_CYCLE(registers[addr]);
							break;
					}
				}
				else					/* nW asserted - this is a write cycle */
				{
					ku8 data = DATA_BUS_PIN;
					TERMINATE_BUS_CYCLE();
					
					cli();
					registers[addr] = data;
					switch(addr)
					{
						case REG_KB_CMD:
							/* Try to start keyboard command transmission */
							ctx_kb.command = data;
							if(ctx_kb.state == state_idle)
								start_tx_keyboard();
							else
								ctx_kb.command_pending = 1;
							break;
							
						case REG_MOUSE_CMD:
							/* Try to start mouse command transmission */
							ctx_mouse.command = data;
							if(ctx_mouse.state == state_idle)
								start_tx_mouse();
							else
								ctx_mouse.command_pending = 1;
							break;
						
						case REG_STATUS:
							if(!(registers[REG_STATUS] & registers[REG_INT_CFG]))
								HOST_IRQ_RELEASE();
							break;
							
						case REG_CFG:
							if(data & CFG_PWR_KB)
								PWR_PORT |= PWR_KB;
							else
								PWR_PORT &= ~PWR_KB;
							
							if(data & CFG_PWR_MOUSE)
								PWR_PORT |= PWR_MOUSE;
							else
								PWR_PORT &= ~PWR_MOUSE;
							break;
					}
					sei();
				}
			}
		}
    }
}
